#include <libs/failure_detector/include/failure_detector.h>

#include <libs/common/include/log.h>

namespace vm_scheduler {

FailureDetector::FailureDetector(TaskStorage* taskStorage, Allocator* allocator)
    : config_(createFailureDetectorConfig())
    , taskStorage_(taskStorage)
    , allocator_(allocator)
{ }

void FailureDetector::handleStaleAllocatingVms_() noexcept
{
    const auto restartStaleAllocatingVmsResult =
        taskStorage_->restartStaleAllocatingVms(
            config_.allocationTimeLimit, config_.common.vmRestartAttemptCount);

    if (restartStaleAllocatingVmsResult.IsFailure()) {
        ERROR() << "Failed to terminate stale allocating VMs: "
                << what(restartStaleAllocatingVmsResult.ErrorRefOrThrow());
    }

    const auto terminateStaleAllocatingVmsResult =
        taskStorage_->terminateStaleAllocatingVms(
            config_.allocationTimeLimit,
            config_.common.vmRestartAttemptCount,
            config_.common.jobRestartAttemptCount);
    if (terminateStaleAllocatingVmsResult.IsFailure()) {
        ERROR() << "Failed to terminate stale allocating VMs: "
                << what(terminateStaleAllocatingVmsResult.ErrorRefOrThrow());
    }
}

void FailureDetector::handleInactiveAgents_() noexcept
{
    const auto terminateInactiveVmsResult =
        taskStorage_->terminateVmsWithInactiveAgents(
            config_.agentInactivityTimeLimit,
            config_.common.jobRestartAttemptCount);
    if (terminateInactiveVmsResult.IsFailure()) {
        ERROR() << "Failed to terminate VMs with inactive agents: "
                << what(terminateInactiveVmsResult.ErrorRefOrThrow());
    }
}

void FailureDetector::handleVmsWithoutAgents_() noexcept
{
    const auto terminateVmsWithoutAgentsResult =
        taskStorage_->terminateVmsWithoutAgents(
            config_.agentStartupTimeLimit, config_.common.jobRestartAttemptCount);
    if (terminateVmsWithoutAgentsResult.IsFailure()) {
        ERROR() << "Failed to terminate VMs without agents: "
                << what(terminateVmsWithoutAgentsResult.ErrorRefOrThrow());
    }
}

void FailureDetector::handleUntrackedVms_() noexcept
{
    allocator_->terminateUntrackedVms();
}

void FailureDetector::cancelTimedOutJobs() noexcept
{
    const auto cancelTimedOutJobsResult
        = taskStorage_->cancelTimedOutJobs();
    if (cancelTimedOutJobsResult.IsFailure()) {
        ERROR() << "Failed to cancel tasks with exceeded time limit: "
                << what(cancelTimedOutJobsResult.ErrorRefOrThrow());
    }
}

void FailureDetector::monitor() noexcept
{
    INFO() << "Failure detector iteration started";
    handleStaleAllocatingVms_();
    handleInactiveAgents_();
    handleVmsWithoutAgents_();
    handleUntrackedVms_();
    cancelTimedOutJobs();
    INFO() << "Failure detector iteration finished";
}

} // namespace vm_scheduler
