#pragma once

#include <zk/client.hpp>

namespace vm_scheduler {

struct ZkConfig {
    std::string lockPath;
    std::vector<char> lockName = {};
};

class DistributedLock {
public:
    DistributedLock(ZkConfig config);
    ~DistributedLock();

    size_t lock();
private:
    void blockUntilAcquired();
//    std::string createdNodeNameAsStr() {
//        std::string s(createdNodeName_.begin(), createdNodeName_.end());
//        return s;
//    }
private:
    zk::client zkClient_;
    ZkConfig config_;
    //    std::mutex mutex_;
    //    std::condition_variable isFirstChild_;
    std::atomic_bool isStopped_{false};
    std::string createdNodeName_;
};

} // namespace vm_scheduler
