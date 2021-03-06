#pragma once

#include "libs/zookeeper/include/zk_config.h"

#include <zk/client.hpp>

#include <memory>

namespace vm_scheduler {

class DistributedLock : public std::enable_shared_from_this<DistributedLock> {
public:
    DistributedLock(ZkConfig config);
    ~DistributedLock();

    void start();
    void stop();
    void lock();
    std::optional<size_t> lockNumber();

private:
    void retryLock(std::future<zk::watch_exists_result>&& f);
    void retryCheckLockAcquired(std::future<zk::watch_exists_result>&& f);
    void checkLockAcquired();
    size_t getZxid(const std::string& nodeName);

private:
    zk::client zkClient_;
    ZkConfig config_;
    std::mutex mutex_;
    std::atomic_bool isStopped_{false};
    std::string createdNodeName_;
    std::optional<size_t> lockNumber_;
};

std::shared_ptr<DistributedLock> createDistLock(bool create);

} // namespace vm_scheduler
