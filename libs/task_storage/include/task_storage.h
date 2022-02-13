#pragma once

#include <libs/common/include/result.h>
#include <libs/common/include/slot.h>
#include <libs/state/include/state.h>

#include <optional>


namespace vm_scheduler {

using PlanId = int64_t;

class TaskStorage {
public:
    TaskStorage() = default;
    virtual ~TaskStorage() = default;

    virtual Result<TaskId> addTask(const TaskParameters& taskParameters) noexcept = 0;
    virtual Result<PlanId> startScheduling(
        const std::string& backendId, const std::chrono::seconds& schedulingInterval) noexcept = 0;
    virtual Result<State> getCurrentState() noexcept = 0;
    virtual Result<void> commitPlanChange(const StateChange& state, const PlanId planId) noexcept = 0;
    virtual Result<std::vector<AllocationPendingVmInfo>> getVmsToAllocate(
        const size_t maxVmAllocationCount) noexcept = 0;
    virtual Result<void> setVmStatus(const VmId id, const VmStatus status) noexcept = 0;
    virtual Result<void> saveVmAllocationResult(const VmId id, const AllocatedVmInfo& allocatedVmInfo) noexcept = 0;
    virtual Result<std::vector<TerminationPendingVmInfo>> getVmsToTerminate(
        const size_t maxVmATerminationCount) noexcept = 0;
};

} // namespace vm_scheduler
