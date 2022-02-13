#pragma once

#include "libs/task_storage/include/task_storage.h"

#include <gmock/gmock.h>


namespace vm_scheduler::testing {

class TaskStorageMock : public TaskStorage {
public:
    using TaskStorage::TaskStorage;

    MOCK_METHOD(Result<TaskId>, addTask, (const TaskParameters& taskParameters), (noexcept, override));
    MOCK_METHOD(Result<PlanId>, startScheduling,
                (const std::string& backendId, const std::chrono::seconds& schedulingInterval), (noexcept, override));
    MOCK_METHOD(Result<State>, getCurrentState, (), (noexcept, override));
    MOCK_METHOD(Result<void>, commitPlanChange, (const StateChange& state, const PlanId planId), (noexcept, override));
    MOCK_METHOD(Result<std::vector<AllocationPendingVmInfo>>, getVmsToAllocate,
                (const size_t maxVmAllocationCount), (noexcept , override));
    MOCK_METHOD(Result<void>, setVmStatus, (const VmId id, const VmStatus status), (noexcept, override));
    MOCK_METHOD(Result<void>, saveVmAllocationResult,
                (const VmId id, const AllocatedVmInfo& allocatedVmInfo), (noexcept, override));
    MOCK_METHOD(Result<std::vector<TerminationPendingVmInfo>>, getVmsToTerminate,
                (const size_t maxVmTerminationCount), (noexcept, override));
};

} // namespace vm_scheduler::testing
