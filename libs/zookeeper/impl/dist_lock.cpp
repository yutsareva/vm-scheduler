#include "libs/zookeeper/include/dist_lock.h"
#include "libs/common/include/log.h"

#include <cassert>
#include <chrono>

using namespace std::chrono_literals;

namespace vm_scheduler {

DistributedLock::DistributedLock(ZkConfig config)
    : zkClient_(zk::client::connect(config.address).get())
    , config_(std::move(config))
{ }

DistributedLock::~DistributedLock() { }

void DistributedLock::start()
{
    lock();
}

void DistributedLock::stop()
{
    isStopped_.store(true);
    zkClient_.close();
    zkClient_.erase(config_.lockPath + "/" + createdNodeName_).get();
}

std::optional<size_t> DistributedLock::lockNumber()
{
    std::unique_lock lock(mutex_);
    return lockNumber_;
}

void DistributedLock::retryLock(std::future<zk::watch_exists_result>&& f)
{
    try {
        f.get().next().get();
    } catch (const std::exception& e) {
        ERROR() << "Error while waiting on exist_watch: " << e.what();
    }
    checkLockAcquired();
}

size_t DistributedLock::getZxid(const std::string& nodeName)
{
    return std::atoi(nodeName.substr(nodeName.size() - 9).c_str());
}

void DistributedLock::retryCheckLockAcquired(
    std::future<zk::watch_exists_result>&& f)
{
    try {
        f.get().next().get();
    } catch (const std::exception& e) {
        ERROR() << "Error while waiting on exist_watch: " << e.what();
    }
    INFO() << "Recheck lock acquired";
    checkLockAcquired();
}

void DistributedLock::lock()
{
    try {
        if (isStopped_) {
            return;
        }
        {
            std::unique_lock lock(mutex_);
            lockNumber_ = std::nullopt;
        }

        auto createdNodeName =
            zkClient_
                .create(
                    config_.lockPath,
                    config_.lockName,
                    zk::create_mode::ephemeral | zk::create_mode::sequential)
                .get()
                .name();
        {
            std::unique_lock lock(mutex_);
            createdNodeName_ = createdNodeName;
        }
        INFO() << "Created node name: " << createdNodeName_;
    } catch (const std::exception& err) {
        ERROR() << "Failed to create node: " << err.what() << ", try again";
        std::this_thread::sleep_for(1s);
        lock();
    }
    checkLockAcquired();
}

void DistributedLock::checkLockAcquired()
{
    try {
        const auto children = zkClient_.get_children("/").get().children();

        INFO() << "Children: ";
        for (const auto& child: children) {
            INFO() << child << ", ";
        }

        std::vector<std::pair<size_t, std::string>> childrenWithSeq;
        for (const auto& child: children) {
            if (child.starts_with(config_.lockPath.substr(1))) {
                childrenWithSeq.emplace_back(getZxid(child), "/" + child);
            }
        }
        std::sort(childrenWithSeq.begin(), childrenWithSeq.end());
        INFO() << "Children: ";
        for (const auto& child: childrenWithSeq) {
            INFO() << child.first << ", " << child.second << "\n";
        }

        std::vector<std::pair<size_t, std::string>>::iterator it;
        {
            std::unique_lock lock(mutex_);
            it = std::find(
                childrenWithSeq.begin(),
                childrenWithSeq.end(),
                std::pair<size_t, std::string>{
                    getZxid(createdNodeName_), createdNodeName_});
            if (it == childrenWithSeq.begin()) {
                INFO() << "Created node is the first";
                auto watchExistsCurrentZNode = zkClient_.watch_exists(it->second);
                std::thread t([self = shared_from_this(),
                               watchExistsCurrentZNode =
                                   std::move(watchExistsCurrentZNode)]() mutable {
                    self->retryCheckLockAcquired(
                        std::move(watchExistsCurrentZNode));
                });
                t.detach();
                lockNumber_ = getZxid(createdNodeName_);
                INFO() << "Lock acquired, set lock number: " << *lockNumber_;
                return;
            }
        }
        INFO() << "Created node is not the first";
        if (it == childrenWithSeq.end()) {
            INFO() << "No created node among children";
            this->lock();
            return;
        }

        auto watchRemoval = zkClient_.watch_exists((it - 1)->second);

        std::thread t([self = shared_from_this(),
                       watchRemoval = std::move(watchRemoval)]() mutable {
            self->retryLock(std::move(watchRemoval));
        });
        t.detach();
    } catch (const std::exception& err) {
        ERROR() << "Failed to check lock acquired: " << err.what();
        checkLockAcquired();
    }
}

std::shared_ptr<DistributedLock> createDistLock(bool create)
{
    if (!create) {
        return nullptr;
    }
    return std::make_shared<DistributedLock>(createZkConfig());
}

} // namespace vm_scheduler
