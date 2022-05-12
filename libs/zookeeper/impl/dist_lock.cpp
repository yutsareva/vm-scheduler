#include "libs/zookeeper/include/dist_lock.h"
#include "libs/common/include/log.h"

#include <cassert>

namespace vm_scheduler {

DistributedLock::DistributedLock(ZkConfig config) : config_(std::move(config))
{
    auto lockFuture = std::async(
        std::launch::async, &DistributedLock::lock, this->shared_from_this());
}

DistributedLock::~DistributedLock() { }

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
        f.get();
    } catch (const std::exception& e) {
        ERROR() << "Error while waiting on exist_watch: " << e.what();
    }
    lock();
}

void DistributedLock::retryCheckLockAcquired(
    std::future<zk::watch_exists_result>&& f)
{
    try {
        f.get();
    } catch (const std::exception& e) {
        ERROR() << "Error while waiting on exist_watch: " << e.what();
    }
    checkLockAcquired();
}

void DistributedLock::lock()
{
    if (isStopped_) {
        return;
    }
    {
        std::unique_lock lock(mutex_);
        lockNumber_ = std::nullopt;
    }

    createdNodeName_ =
        zkClient_
            .create(
                config_.lockPath,
                config_.lockName,
                zk::create_mode::ephemeral | zk::create_mode::sequential)
            .get()
            .name();
}

void DistributedLock::checkLockAcquired()
{
    while (!isStopped_.load()) {
        const auto children =
            zkClient_.get_children(config_.lockPath).get().children();
        std::vector<std::pair<size_t, std::string>> childrenWithSeq;
        std::transform(
            children.begin(),
            children.end(),
            std::back_inserter(childrenWithSeq),
            [](const auto& childName) -> std::pair<size_t, std::string> {
                return {std::atoi(childName.c_str()), childName};
            });
        std::sort(childrenWithSeq.begin(), childrenWithSeq.end());

        const auto it = std::find(
            childrenWithSeq.begin(),
            childrenWithSeq.end(),
            std::pair<size_t, std::string>{
                std::atoi(createdNodeName_.c_str()), createdNodeName_});
        if (it == childrenWithSeq.begin()) {
            auto watchExistsCurrentZNode =
                zkClient_.watch_exists(config_.lockPath + "/" + it->second);
            std::async([self = shared_from_this(),
                        watchExistsCurrentZNode =
                            std::move(watchExistsCurrentZNode)]() mutable {
                self->retryCheckLockAcquired(std::move(watchExistsCurrentZNode));
            });
            std::unique_lock lock(mutex_);
            lockNumber_ = std::atoi(createdNodeName_.c_str());
            return;
        }

        auto watchRemoval =
            zkClient_.watch_exists(config_.lockPath + "/" + (it - 1)->second);

        std::async([self = shared_from_this(),
                    watchRemoval = std::move(watchRemoval)]() mutable {
            self->retryLock(std::move(watchRemoval));
        });
    }
}

ZkConfig createZkConfig()
{
    return ZkConfig{
        // TODO
    };
}

std::shared_ptr<DistributedLock> createDistLock(bool create)
{
    if (!create) {
        return nullptr;
    }
    return std::make_shared<DistributedLock>(createZkConfig());
}

} // namespace vm_scheduler
