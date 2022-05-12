#pragma once

#include <libs/task_storage/include/task_storage.h>

#include <libs/postgres/include/pg_pool.h>

#include <pqxx/pqxx>

#include <optional>

namespace vm_scheduler {

class PgTaskStorage : public TaskStorage {
public:
    explicit PgTaskStorage(pg::PgPool&& pool) noexcept;
    ~PgTaskStorage() override = default;

    Result<CreatedJobs> addTask(
        const TaskParameters& taskParameters) noexcept override;
    Result<PlanId> startScheduling(
        const std::string& backendId,
        const std::chrono::seconds& schedulingInterval) noexcept override;
    Result<State> getCurrentState() noexcept override;
    Result<void> commitPlanChange(
        const StateChange& state, const PlanId planId) noexcept override;
    Result<std::vector<AllocationPendingVmInfo>> getVmsToAllocate(
        const size_t maxVmAllocationCount) noexcept override;
    Result<std::vector<TerminationPendingVmInfo>> getVmsToTerminate(
        const size_t maxVmTerminationCount) noexcept override;
    Result<void> rollbackUnallocatedVmsState(
        const std::vector<VmId>& vmIds,
        const size_t vmRestartAttemptCount,
        const size_t jobRestartAttemptCount) noexcept override;
    Result<void> returnUnterminatedVms(
        const std::vector<VmId>& vmIds) noexcept override;
    Result<void> saveVmAllocationResult(
        const VmId id, const AllocatedVmInfo& allocatedVmInfo) noexcept override;
    Result<void> saveVmTerminationResult(const VmId vmId) noexcept override;
    Result<JobStates> getJobStates(const TaskId id) noexcept override;
    Result<void> cancelTask(const TaskId id) noexcept override;
    Result<void> restartStaleAllocatingVms(
        const std::chrono::seconds& allocationTimeLimit,
        const size_t vmRestartAttemptCount) noexcept override;
    Result<void> terminateStaleAllocatingVms(
        const std::chrono::seconds& allocationTimeLimit,
        const size_t vmRestartAttemptCount,
        const size_t jobRestartAttemptCount) noexcept override;
    Result<void> terminateVmsWithInactiveAgents(
        const std::chrono::seconds& agentInactivityTimeLimit,
        const size_t jobRestartAttemptCount) noexcept override;
    Result<void> terminateVmsWithoutAgents(
        const std::chrono::seconds& agentStartupTimeLimit,
        const size_t jobRestartAttemptCount) noexcept override;
    Result<std::vector<AssignedJob>> getAssignedJobs(
        const VmId vmId) noexcept override;
    Result<JobToLaunch> getJobToLaunch(
        const VmId vmId, const JobId jobId) noexcept override;
    Result<void> updateJobState(
        const VmId vmId,
        const JobId jobId,
        const JobState& jobState) noexcept override;
    Result<AllocatedVmInfos> getAllocatedVms() noexcept override;

private:
    Result<bool> isSchedulingRelevant_(
        pqxx::transaction_base& txn, const PlanId currentPlanId) noexcept;
    Result<void> refreshPlanActivityTs_(
        pqxx::transaction_base& txn, const PlanId currentPlanId) noexcept;
    Result<pg::TransactionHandle> acquireLock_(const bool noWait) noexcept;
    Result<bool> planWasRecentlyUpdated_(
        pqxx::transaction_base& txn,
        const std::chrono::seconds& schedulingInterval) noexcept;
    Result<PlanId> insertNewPlanRecord_(
        pqxx::transaction_base& txn, const BackendId& backendId) noexcept;
    Result<void> terminateVms_(
        pqxx::transaction_base& txn,
        const std::vector<VmId>& vmsToTerminate) noexcept;
    Result<void> updateVmCapacities_(
        pqxx::transaction_base& txn,
        const VmIdToCapacity& updatedIdleCapacities) noexcept;
    Result<std::unordered_map<DesiredSlotId, VmId>> allocateNewVms_(
        pqxx::transaction_base& txn, const DesiredSlotMap& desiredSlots) noexcept;
    Result<void> applyJobVmAssignments_(
        pqxx::transaction_base& txn,
        const JobToVm& vmAssignments,
        std::unordered_map<DesiredSlotId, VmId> slotsToVms) noexcept;
    Result<std::vector<QueuedJobInfo>> getQueuedJobs_(
        pqxx::transaction_base& txn) noexcept;
    Result<std::vector<ActiveVm>> getActiveVms_(pqxx::transaction_base& txn) noexcept;
    Result<void> setVmStatus_(
        const std::vector<VmId>& vmIds, const VmStatus status) noexcept;
    Result<pg::TransactionHandle> readOnlyTransaction_() noexcept;
    Result<pqxx::result> execWritableQuery_(const std::string& query) noexcept;
    Result<void> restartAllocatingVms_(
        const std::string& restartCondition,
        const size_t vmRestartAttemptCount) noexcept;
    Result<void> terminateVms_(
        const std::string& terminateCondition,
        const VmStatus initialStatus,
        const VmStatus targetStatus,
        const size_t vmRestartAttemptCount,
        const size_t jobRestartAttemptCount) noexcept;
    Result<void> cancelTimedOutJobs() noexcept override;

private:
    pg::PgPool pool_;
};

} // namespace vm_scheduler
