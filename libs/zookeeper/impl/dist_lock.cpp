#include "libs/zookeeper/include/dist_lock.h"

#include <cassert>

namespace vm_scheduler {

DistributedLock::DistributedLock(ZkConfig config)
    : config_(std::move(config)){};
DistributedLock::~DistributedLock()
{
    isStopped_.store(true);
    zkClient_.erase(config_.lockPath + "/" + createdNodeName_).get();
};

size_t DistributedLock::lock()
{
    createdNodeName_ =
        zkClient_
            .create(
                config_.lockPath,
                config_.lockName,
                zk::create_mode::ephemeral | zk::create_mode::sequential)
            .get()
            .name();

    blockUntilAcquired();
    return createdNodeName_;
}

void DistributedLock::blockUntilAcquired()
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
            return;
        }

        auto watchRemoval =
            zkClient_.watch_exists(config_.lockPath + "/" + (it - 1)->second).get();
        assert(watchRemoval.initial());

        //        auto handleRemoval = [&createdNode](const std::future<zk::event>& event) {
        //        };
        //                future.next().then(createdNode);
        watchRemoval.next().get();
    }
}

} // namespace vm_scheduler
