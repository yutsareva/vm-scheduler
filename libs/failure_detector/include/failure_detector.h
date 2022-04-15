#pragma once

#include <libs/failure_detector/include/config.h>
#include <libs/task_storage/include/task_storage.h>

#include <libs/concurrency/include/background_thread.h>


namespace vm_scheduler {

class FailureDetector {
public:
    FailureDetector(TaskStorage* taskStorage);
    void monitor() noexcept;

private:
    void handleStaleAllocatingVms_() noexcept;
    void handleInactiveAgents_() noexcept;
    void handleVmsWithoutAgents_() noexcept;

private:
    FailureDetectorConfig config_;
    TaskStorage* taskStorage_;
};

} // namespace maps::b2bgeo::vm_scheduler
