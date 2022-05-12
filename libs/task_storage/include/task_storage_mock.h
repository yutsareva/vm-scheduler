#pragma once

#include "libs/task_storage/include/task_storage.h"

#include <libs/state/include/task.h>

#include <gmock/gmock.h>

namespace vm_scheduler::testing {

class TaskStorageMock : public TaskStorage {
public:
    using TaskStorage::TaskStorage;

    MOCK_METHOD(
        Result<CreatedJobs>,
        addTask,
        (const TaskParameters& taskParameters),
        (noexcept, override));
    MOCK_METHOD(
        Result<PlanId>,
        startScheduling,
        (const std::string& backendId,
         const std::chrono::seconds& schedulingInterval),
        (noexcept, override));
    MOCK_METHOD(Result<State>, getCurrentState, (), (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        commitPlanChange,
        (const StateChange& state, const PlanId planId),
        (noexcept, override));
    MOCK_METHOD(
        Result<std::vector<AllocationPendingVmInfo>>,
        getVmsToAllocate,
        (const size_t maxVmAllocationCount),
        (noexcept, override));
    MOCK_METHOD(
        Result<std::vector<TerminationPendingVmInfo>>,
        getVmsToTerminate,
        (const size_t maxVmTerminationCount),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        rollbackUnallocatedVmsState,
        (const std::vector<VmId>& vmIds,
         const size_t vmRestartAttemptCount,
         const size_t jobRestartAttemptCount),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        returnUnterminatedVms,
        (const std::vector<VmId>& vmIds),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        saveVmAllocationResult,
        (const VmId id, const AllocatedVmInfo& allocatedVmInfo),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        saveVmTerminationResult,
        (const VmId id),
        (noexcept, override));
    MOCK_METHOD(
        Result<JobStates>, getJobStates, (const TaskId id), (noexcept, override));
    MOCK_METHOD(
        Result<void>, cancelTask, (const TaskId taskId), (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        restartStaleAllocatingVms,
        (const std::chrono::seconds& allocationTimeLimit,
         const size_t vmRestartAttemptCount),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        terminateStaleAllocatingVms,
        (const std::chrono::seconds& allocationTimeLimit,
         const size_t vmRestartAttemptCount,
         const size_t jobRestartAttemptCount),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        terminateVmsWithInactiveAgents,
        (const std::chrono::seconds& agentInactivityTimeLimit,
         const size_t jobRestartAttemptCount),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        terminateVmsWithoutAgents,
        (const std::chrono::seconds& agentStartupTimeLimit,
         const size_t jobRestartAttemptCount),
        (noexcept, override));
    MOCK_METHOD(
        Result<std::vector<AssignedJob>>,
        getAssignedJobs,
        (const VmId vmId),
        (noexcept, override));
    MOCK_METHOD(
        Result<JobToLaunch>,
        getJobToLaunch,
        (const VmId vmId, const JobId jobId),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>,
        updateJobState,
        (const VmId vmId, const JobId jobId, const JobState& jobState),
        (noexcept, override));
    MOCK_METHOD(
        Result<AllocatedVmInfos>, getAllocatedVms, (), (noexcept, override));
    MOCK_METHOD(Result<void>, cancelTimedOutJobs, (), (noexcept, override));
};

} // namespace vm_scheduler::testing
