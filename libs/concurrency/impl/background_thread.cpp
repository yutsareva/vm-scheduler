#include <libs/concurrency/include/background_thread.h>

namespace vm_scheduler {

BackgroundThread::BackgroundThread(std::function<void()> payload, std::chrono::seconds sleepDuration)
    : payload_(std::move(payload)), sleepDuration_(sleepDuration), isStopped_(false)
{
    thread_ = std::thread(&BackgroundThread::runner, this);
}

BackgroundThread::~BackgroundThread()
{
    isStopped_.store(false);
    isAwake_.notify_one();
    thread_.join();
}

void BackgroundThread::runner()
{
    while (!isStopped_) {
        auto start = std::chrono::steady_clock::now();
        payload_();
        std::unique_lock<std::mutex> lock(mutex_);
        while (!isStopped_ || (start + sleepDuration_ > std::chrono::steady_clock::now())) {
            isAwake_.wait_until(lock, start + sleepDuration_);
        }
    }
}

} // namespace vm_scheduler
