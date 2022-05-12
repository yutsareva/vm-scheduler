#pragma once

#include <zk/client.hpp>

#include <memory>

namespace vm_scheduler {

struct ZkConfig {
    std::string lockPath;
    std::vector<char> lockName = {};
};

class DistributedLock : std::enable_shared_from_this<DistributedLock> {
public:
    DistributedLock(ZkConfig config);
    ~DistributedLock();

    void stop();
    void lock();
    std::optional<size_t> lockNumber();

private:
    void retryLock(std::future<zk::watch_exists_result>&& f);
    void retryCheckLockAcquired(std::future<zk::watch_exists_result>&& f);
    void checkLockAcquired();

private:
    zk::client zkClient_;
    ZkConfig config_;
    std::mutex mutex_;
    std::atomic_bool isStopped_{false};
    std::string createdNodeName_;
    std::optional<size_t> lockNumber_;
};

ZkConfig createZkConfig();

std::shared_ptr<DistributedLock> createDistLock(bool create);

} // namespace vm_scheduler
