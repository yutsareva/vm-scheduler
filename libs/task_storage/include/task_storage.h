#pragma once

#include <libs/common/include/result.h>
#include <libs/common/include/slot.h>
#include <libs/state/include/state.h>

#include <chrono>
#include <optional>

namespace vm_scheduler {

using PlanId = int64_t;

class TaskStorage {
public:
    TaskStorage() = default;
    virtual ~TaskStorage() = default;

    virtual Result<CreatedJobs> addTask(
        const TaskParameters& taskParameters) noexcept = 0;
    virtual Result<PlanId> startScheduling(
        const std::string& backendId,
        const std::chrono::seconds& schedulingInterval) noexcept = 0;
    virtual Result<State> getCurrentState() noexcept = 0;
    virtual Result<void> commitPlanChange(
        const StateChange& state, const PlanId planId) noexcept = 0;
    virtual Result<std::vector<AllocationPendingVmInfo>> getVmsToAllocate(
        const size_t maxVmAllocationCount) noexcept = 0;
    virtual Result<std::vector<TerminationPendingVmInfo>> getVmsToTerminate(
        const size_t maxVmATerminationCount) noexcept = 0;
    virtual Result<void> rollbackUnallocatedVmsState(
        const std::vector<VmId>& vmIds,
        const size_t vmRestartAttemptCount,
        const size_t jobRestartAttemptCount) noexcept = 0;
    virtual Result<void> returnUnterminatedVms(
        const std::vector<VmId>& vmIds) noexcept = 0;
    virtual Result<void> saveVmAllocationResult(
        const VmId id, const AllocatedVmInfo& allocatedVmInfo) noexcept = 0;
    virtual Result<void> saveVmTerminationResult(const VmId id) noexcept = 0;
    virtual Result<JobStates> getJobStates(const TaskId id) noexcept = 0;
    virtual Result<void> cancelTask(const TaskId taskId) noexcept = 0;
    virtual Result<void> restartStaleAllocatingVms(
        const std::chrono::seconds& allocationTimeLimit,
        const size_t vmRestartAttemptCount) noexcept = 0;
    virtual Result<void> terminateStaleAllocatingVms(
        const std::chrono::seconds& allocationTimeLimit,
        const size_t vmRestartAttemptCount,
        const size_t jobRestartAttemptCount) noexcept = 0;
    virtual Result<void> terminateVmsWithInactiveAgents(
        const std::chrono::seconds& agentInactivityTimeLimit,
        const size_t jobRestartAttemptCount) noexcept = 0;
    virtual Result<void> terminateVmsWithoutAgents(
        const std::chrono::seconds& agentStartupTimeLimit,
        const size_t jobRestartAttemptCount) noexcept = 0;
    virtual Result<std::vector<AssignedJob>> getAssignedJobs(
        const VmId vmId) noexcept = 0;
    virtual Result<JobToLaunch> getJobToLaunch(
        const VmId vmId, const JobId jobId) noexcept = 0;
    virtual Result<void> updateJobState(
        const VmId vmId, const JobId jobId, const JobState& jobState) noexcept = 0;
    virtual Result<AllocatedVmInfos> getAllocatedVms() noexcept = 0;
    virtual Result<void> cancelTimedOutJobs() noexcept = 0;
};

} // namespace vm_scheduler
