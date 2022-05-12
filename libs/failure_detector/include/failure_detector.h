#pragma once

#include <libs/failure_detector/include/config.h>
#include <libs/task_storage/include/task_storage.h>
#include <libs/allocator/include/allocator.h>

#include <libs/concurrency/include/background_thread.h>

namespace vm_scheduler {

class FailureDetector {
public:
    FailureDetector(TaskStorage* taskStorage, Allocator* allocator);
    void monitor() noexcept;

private:
    void handleStaleAllocatingVms_() noexcept;
    void handleInactiveAgents_() noexcept;
    void handleVmsWithoutAgents_() noexcept;
    void handleUntrackedVms_() noexcept;
    void cancelTimedOutJobs() noexcept;

private:
    FailureDetectorConfig config_;
    TaskStorage* taskStorage_;
    Allocator* allocator_;
};

} // namespace vm_scheduler
