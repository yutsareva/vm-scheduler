#pragma once

#include <atomic>
#include <condition_variable>
#include <chrono>
#include <functional>

namespace vm_scheduler {

class BackgroundThread {
public:
    BackgroundThread(std::function<void()> payload, std::chrono::seconds sleepDuration);

    ~BackgroundThread();

private:
    void runner();

    std::function<void()> payload_;
    std::chrono::seconds sleepDuration_;
    std::atomic<bool> isStopped_;
    std::mutex mutex_;
    std::condition_variable isAwake_;
    std::thread thread_;
};

} // namespace vm_scheduler
