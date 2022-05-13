#include "libs/db/include/pg_task_storage.h"
#include "libs/db/impl/errors.h"

#include <libs/state/include/task.h>
#include <libs/state/include/errors.h>

#include <libs/common/include/stringify.h>
#include <libs/postgres/include/helpers.h>

#include <libs/common/include/log.h>

#include <exception>
//#include <format>
#include <sstream>


namespace vm_scheduler {

PgTaskStorage::PgTaskStorage(pg::PgPool&& pool) noexcept
    : pool_(std::move(pool))
{
}

Result<pqxx::result> PgTaskStorage::execWritableQuery_(const std::string& query) noexcept
{
    try {
        auto txn = pool_.writableTransaction();
        auto result = pg::execQuery(query, *txn);
        txn->commit();
        return Result{std::move(result)};
    } catch (const std::exception& ex) {
        return Result<pqxx::result>::Failure<PgException>(toString(
            "Unexpected pg exception: ", ex.what()));
    }
}

Result<CreatedJobs> PgTaskStorage::addTask(const TaskParameters& taskParameters) noexcept
{
    try {
        auto txn = pool_.writableTransaction();
        const auto createTaskQuery = toString(
            "INSERT INTO scheduler.tasks (job_count, client_id, settings, image_version) ",
            "VALUES(", taskParameters.jobCount, ", ", txn->quote(taskParameters.clientId), ", ",
            txn->quote(taskParameters.settings), ", ", txn->quote(taskParameters.imageVersion), ") ",
            "RETURNING id;"
        );
        const auto createTask = pg::execQuery(createTaskQuery, *txn);
        const auto taskId = createTask[0].at("id").as<TaskId>();

        // TODO: replace with std::format
//        const auto addJobsQuery = toString(
//            "INSERT INTO scheduler.jobs ",
//            "(task_id, cpu, ram, estimation, status, created, options) ",
//            "VALUES {} RETURNING id;");
        const auto addJobsQuery =
            "INSERT INTO scheduler.jobs "
            "(task_id, cpu, ram, estimation, status, created, options) "
            "VALUES ";
        // TODO: replace with std::format
//        const auto addJobValue = toString(
//            "({}, ", taskParameters.requiredCapacity.cpu.count(), ", ", taskParameters.requiredCapacity.ram.count(),
//            ", interval '", taskParameters.estimation.count(), " seconds', '",
//            toString(JobStatus::Queued), "', NOW(), {})"
//        );
        const auto addJobValue = toString(
            ", ", taskParameters.requiredCapacity.cpu.count(), ", ", taskParameters.requiredCapacity.ram.count(),
            ", interval '", taskParameters.estimation.count(), " seconds', '",
            toString(JobStatus::Queued), "', NOW(), "
        );

        std::vector<std::string> addJobValuesFormatted;
        addJobValuesFormatted.reserve(taskParameters.jobOptions.size());
        std::transform(taskParameters.jobOptions.begin(), taskParameters.jobOptions.end(),
                       std::back_inserter(addJobValuesFormatted),
                       [&addJobValue, &taskId, &txn](const JobOptions& jobOptions) {
                           // TODO: replace with std::format
                           // return std::format(addJobValue, taskId, txn->quote(jobOptions));
                           return toString(
                               "(", taskId, addJobValue, txn->quote(jobOptions), ")"
                            );
                       });

        const auto addJobsValuesConcatted =  joinSeq(addJobValuesFormatted);
        // TODO: replace with std::format
        // const auto addJobsQueryFormatted = std::format(addJobsQuery, addJobsValuesConcatted);
        const auto addJobsQueryFormatted = toString(
            addJobsQuery, addJobsValuesConcatted, "RETURNING id;"
        );
        const auto addedJobs = pg::execQuery(addJobsQueryFormatted, *txn);
        txn->commit();

        std::vector<JobId> jobIds;
        jobIds.reserve(addedJobs.size());
        std::transform(addedJobs.begin(), addedJobs.end(), std::back_inserter(jobIds),
                       [](const pqxx::row& row) -> JobId { return row.at("id").as<JobId>(); });
        return Result{CreatedJobs{
            .taskId = taskId,
            .jobIds = std::move(jobIds),
        }};
    } catch (const std::exception& ex) {
        ERROR() << ex.what();
        return Result<CreatedJobs>::Failure<PgException>(toString(
            "Unexpected exception while inserting new job: ", ex.what()));
    }
}

Result<PlanId> PgTaskStorage::startScheduling(
    const std::string& backendId,
    const std::chrono::seconds& schedulingInterval,
    const std::optional<size_t>& lockNumber) noexcept
{
    const bool noWait = !lockNumber;
    auto txnResult = acquireLock_(noWait);
    if (txnResult.IsFailure()) {
        return Result<PlanId>::Failure(std::move(txnResult).ErrorOrThrow());
    }
    auto handler = std::move(txnResult).ValueOrThrow();
    auto& txn = *(handler);
    if (lockNumber) {
        checkLeader_(txn, *lockNumber);
    }

    auto recentlyUpdatedResult = planWasRecentlyUpdated_(txn, schedulingInterval);
    if (recentlyUpdatedResult.IsFailure()) {
        return Result<PlanId>::Failure(std::move(recentlyUpdatedResult).ErrorOrThrow());
    } else if (recentlyUpdatedResult.ValueRefOrThrow()) {
        txn.commit();
        return Result<PlanId>::Failure<SchedulingCancelled>(
            "The plan was recently updated, new scheduling is not needed yet");
    }

    return insertNewPlanRecord_(txn, backendId);
}

Result<State> PgTaskStorage::getCurrentState() noexcept
{
    auto txnResult = readOnlyTransaction_();
    if (txnResult.IsFailure()) {
        return Result<State>::Failure(std::move(txnResult).ErrorOrThrow());
    }
    auto handler = std::move(txnResult).ValueOrThrow();
    auto& txn = *(handler);

    auto queuedJobsResult = getQueuedJobs_(txn);
    if (queuedJobsResult.IsFailure()) {
        return Result<State>::Failure(std::move(queuedJobsResult).ErrorOrThrow());
    }

    auto activeVmsResult = getActiveVms_(txn);
    if (activeVmsResult.IsFailure()) {
        return Result<State>::Failure(std::move(activeVmsResult).ErrorOrThrow());
    }

    return Result{State{
        .queuedJobs = std::move(queuedJobsResult).ValueOrThrow(),
        .vms = std::move(activeVmsResult).ValueOrThrow(),
    }};
}

Result<void> PgTaskStorage::commitPlanChange(
    const StateChange& state,
    const PlanId planId) noexcept
{
    const bool noWait = false;
    auto txnResult = acquireLock_(noWait);
    if (txnResult.IsFailure()) {
        return Result<void>::Failure(std::move(txnResult).ErrorOrThrow());
    }
    auto handler = std::move(txnResult).ValueOrThrow();
    auto& txn = *(handler);

    auto relevantResult = isSchedulingRelevant_(txn, planId);
    if (relevantResult.IsFailure()) {
        return Result<void>::Failure(std::move(relevantResult).ErrorOrThrow());
    } else if (!relevantResult.ValueRefOrThrow()) {
        return Result<void>::Failure<SchedulingCancelled>(
            "Current scheduling is not the newest one");
    }

    const auto terminateResult = terminateVms_(txn, state.vmsToTerminate);
    if (terminateResult.IsFailure()) {
        return Result<void>::Failure(terminateResult.ErrorRefOrThrow());
    }

    const auto vmUpdates = updateVmCapacities_(txn, state.updatedIdleCapacities);
    if (vmUpdates.IsFailure()) {
        return Result<void>::Failure(vmUpdates.ErrorRefOrThrow());
    }

    const auto allocatedVms = allocateNewVms_(txn, state.desiredSlotMap);
    if (allocatedVms.IsFailure()) {
        return Result<void>::Failure(allocatedVms.ErrorRefOrThrow());
    }

    const auto assignmentsResult = applyJobVmAssignments_(txn, state.jobToVm, allocatedVms.ValueRefOrThrow());
    if (assignmentsResult.IsFailure()) {
        return Result<void>::Failure(assignmentsResult.ErrorRefOrThrow());
    }

    const auto activityRefreshResult = refreshPlanActivityTs_(txn, planId);
    if (activityRefreshResult.IsFailure()) {
        return Result<void>::Failure(activityRefreshResult.ErrorRefOrThrow());
    }

    try {
        txn.commit();
        return Result<void>::Success();
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString(
            "Unexpected exception while committing plan change: ", ex.what()));
    }
}

Result<bool> PgTaskStorage::isSchedulingRelevant_(pqxx::transaction_base& txn, const PlanId currentPlanId) noexcept
{
    const auto lastSchedulingQuery = "SELECT id FROM scheduler.plan WHERE updated = (SELECT MAX(updated) FROM scheduler.plan);";
    const auto lastScheduling = pg::execQuery(lastSchedulingQuery, txn);

    if (lastScheduling.affected_rows() != 1) {
        return Result<bool>::Failure<InconsistentPgResultException>(toString(
            "Invalid number of relevant schedulings: ", lastScheduling.affected_rows()));
    }

    const auto schedulingIsRelevant = lastScheduling[0].at("id").as<PlanId>() == currentPlanId;
    if (!schedulingIsRelevant) {
        ERROR() << "The newest scheduling id is " << lastScheduling[0].at("id").as<PlanId>()
                << ", while current scheduling id is " << currentPlanId;
    }
    return Result{schedulingIsRelevant};
}

Result<void> PgTaskStorage::refreshPlanActivityTs_(pqxx::transaction_base& txn, const PlanId currentPlanId) noexcept
{
    try {
        const auto updatePlanQuery = toString(
            "UPDATE scheduler.plan SET updated = NOW() WHERE id = ", currentPlanId, ";");
        pg::execQuery(updatePlanQuery, txn);
        return Result<void>::Success();
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString(
            "Unexpected exception while refreshing plan activity: ", ex.what()));
    }
}

Result<pg::TransactionHandle> PgTaskStorage::acquireLock_(const bool noWait) noexcept
{
    const auto lockQuery = toString("LOCK TABLE scheduler.plan IN SHARE ROW EXCLUSIVE MODE", noWait ? " NOWAIT;" : ";");
    try {
        auto txn = pool_.writableTransaction();
        pg::execQuery(lockQuery, *txn);
        return Result{std::move(txn)};
    } catch (const pqxx::sql_error& ex) {
        if (strstr(ex.what(), "could not obtain lock") != nullptr) {
            return Result<pg::TransactionHandle>::Failure<LockUnavailable>(toString(
                "Failed to obtain SHARE ROW EXCLUSIVE lock on scheduler.plan table",  noWait ? " with NOWAIT;" : ""));
        }
        return Result<pg::TransactionHandle>::Failure<PgException>(toString(
            "Unexpected sql error while acquiring lock: ", ex.what()));
    } catch (const std::exception& ex) {
        return Result<pg::TransactionHandle>::Failure<PgException>(toString(
            "Unexpected exception while acquiring lock: ", ex.what()));
    }
}

Result<void> checkLeader_(
    pqxx::transaction_base& txn, const size_t lockNumber) noexcept
{
    const auto checkLeaderQuery =
        "SELECT COALESCE(MAX(id), 0) as max_id FROM scheduler.locks;";
    try {
        const auto result = pg::execQuery(checkLeaderQuery, txn);
        const auto maxId = result[0].at("max_id").as<size_t>();
        if (lockNumber != maxId) {
            INFO() << "Max lock number: " << maxId << ", "
                   << "current lock number: " << lockNumber;
            if (lockNumber > maxId) {
                INFO() << "Inserting new lock number: " << lockNumber;
                const auto insertNewLockNumberQuery =
                    toString("INSERT INTO scheduler.locks (max_id) VALUES (", lockNumber, ");");
                pg::execQuery(insertNewLockNumberQuery, txn);
            }
        }
        return Result<void>::Success();
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString(
            "Unexpected exception while checking lock number: ", ex.what()));
    }
}

Result<bool> PgTaskStorage::planWasRecentlyUpdated_(
    pqxx::transaction_base& txn, const std::chrono::seconds& schedulingInterval) noexcept
{
    const auto lastSchedulerActivityQuery =
        "SELECT extract(epoch from COALESCE(MAX(updated), to_timestamp(0))) as last_updated, "
        "extract(epoch from NOW()) as now FROM scheduler.plan;";
    try {
        const auto result = pg::execQuery(lastSchedulerActivityQuery, txn);
        const auto planWasRecentlyUpdated_ =
            result[0].at("last_updated").as<double>() + schedulingInterval.count() > result[0].at("now").as<double>();
        return Result{planWasRecentlyUpdated_};
    } catch (const std::exception& ex) {
        return Result<bool>::Failure<PgException>(toString(
            "Unexpected exception while checking last updated plan: ", ex.what()));
    }
}

Result<PlanId> PgTaskStorage::insertNewPlanRecord_(pqxx::transaction_base& txn, const BackendId& backendId) noexcept
{
    try {
        const auto newPlanQuery = toString(
            "INSERT INTO scheduler.plan (created, updated, backend_id) "
            "VALUES(NOW(), NOW(), ", txn.quote(backendId), ") RETURNING id;");
        const auto newPlanResult = pg::execQuery(newPlanQuery, txn);
        txn.commit();
        return Result{newPlanResult[0].at("id").as<PlanId>()};
    } catch (const std::exception& ex) {
        return Result<PlanId>::Failure<PgException>(toString(
            "Unexpected exception while inserting new plan record: ", ex.what()));
    }
}

Result<void> PgTaskStorage::terminateVms_(
    pqxx::transaction_base& txn, const std::vector<VmId>& vmsToTerminate) noexcept
{
    if (vmsToTerminate.empty()) {
        return Result<void>::Success();
    }

    const auto terminateVmsQuery = toString(
        "UPDATE scheduler.vms SET status = '", toString(VmStatus::PendingTermination), "' ",
        "WHERE id IN ", pg::asFormattedList(vmsToTerminate, txn), ";");
    try {
        pg::execQuery(terminateVmsQuery, txn);
        return Result<void>::Success();
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString("Unexpected exception while setting status '",
                                                           toString(VmStatus::PendingTermination), "' for vms: ", ex.what()));
    }
}

Result<void> PgTaskStorage::updateVmCapacities_(
    pqxx::transaction_base& txn, const VmIdToCapacity& updatedIdleCapacities) noexcept
{
    if (updatedIdleCapacities.empty()) {
        return Result<void>::Success();
    }

    std::stringstream updateVmsQuery;
    updateVmsQuery << "UPDATE scheduler.vms AS v SET cpu_idle = n.cpu_idle, ram_idle = n.ram_idle FROM (VALUES ";
    for (auto it = updatedIdleCapacities.begin(); it != updatedIdleCapacities.end(); ++it) {
        updateVmsQuery << (it != updatedIdleCapacities.begin() ? ", " : "");
        updateVmsQuery << "(" << it->second.cpu.count() << ", "
                       <<  it->second.ram.count() << ", " << it->first << ")";
    }
    updateVmsQuery << ") AS n(cpu_idle, ram_idle, id) WHERE v.id = n.id;";

    try {
        pg::execQuery(updateVmsQuery.str(), txn);
        return Result<void>::Success();
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString(
            "Unexpected exception while updating vms' capacities: ", ex.what()));
    }
}

Result<std::unordered_map<DesiredSlotId, VmId>> PgTaskStorage::allocateNewVms_(
    pqxx::transaction_base& txn, const DesiredSlotMap& desiredSlots) noexcept
{
    std::unordered_map<DesiredSlotId, VmId> allocatedVms;
    for (const auto& [desiredSlotId, slot] : desiredSlots) {
        const auto addVmsQuery = toString(
            "INSERT INTO scheduler.vms (status, cpu, ram, cpu_idle, ram_idle, created, last_status_update) ",
            "VALUES('", toString(VmStatus::PendingAllocation), "', ",
            slot.total.cpu.count(), ", ", slot.total.ram.count(), ", ",
            slot.idle.cpu.count(), ", ", slot.idle.ram.count(), ", NOW(), NOW()) RETURNING id;");

        try {
            const auto result = pg::execQuery(addVmsQuery, txn);
            allocatedVms.emplace(desiredSlotId, result[0].at("id").as<VmId>());
        } catch (const std::exception& ex) {
            return Result<std::unordered_map<DesiredSlotId, VmId>>::Failure<PgException>(toString(
                "Unexpected exception while inserting new vm row: ", ex.what()));
        }
    }

    return Result{std::move(allocatedVms)};
}

Result<void> PgTaskStorage::applyJobVmAssignments_(
    pqxx::transaction_base& txn, const JobToVm& vmAssignments, std::unordered_map<DesiredSlotId, VmId> slotsToVms) noexcept
{
    if (vmAssignments.empty()) {
        return Result<void>::Success();
    }

    std::stringstream tmpTable;
    for (auto it = vmAssignments.begin(); it != vmAssignments.end(); ++it) {
        if (it != vmAssignments.begin()) {
            tmpTable << ", ";
        }
        if (std::holds_alternative<DesiredSlotId>(it->second)) {
            tmpTable << "(" << it->first << ", " << slotsToVms[std::get<DesiredSlotId>(it->second)] << ")";
        } else {
            tmpTable << "(" << it->first << ", " << std::get<VmId>(it->second) << ")";
        }
    }

    const auto updateJobQuery = toString(
        "UPDATE scheduler.jobs AS t SET ",
        "status = '", toString(JobStatus::Scheduled), "', "
        "vm_id = a.vm_id ",
        "FROM (VALUES ",
        tmpTable.str(),
        ") AS a(job_id, vm_id) ",
        "WHERE id = a.job_id;");

    try {
        pg::execQuery(updateJobQuery, txn);
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString(
            "Unexpected exception while setting status '", toString(JobStatus::Scheduled),
            "' for jobs: ", ex.what()));
    }
    return Result<void>::Success();
}

Result<std::vector<QueuedJobInfo>> PgTaskStorage::getQueuedJobs_(pqxx::transaction_base& txn) noexcept
{
    const auto queuedJobsQuery = toString(
        "SELECT id, cpu, ram FROM scheduler.jobs WHERE status = '", toString(JobStatus::Queued), "';");
    try {
        const auto jobsResult = pg::execQuery(queuedJobsQuery, txn);

        std::vector<QueuedJobInfo> queuedJobs;
        queuedJobs.reserve(jobsResult.affected_rows());
        for (const auto& row : jobsResult) {
            queuedJobs.push_back(QueuedJobInfo{
                .id = row.at("id").as<JobId>(),
                .requiredCapacity = {
                    .cpu = CpuCores(row.at("cpu").as<size_t>()),
                    .ram = MegaBytes(row.at("ram").as<size_t>()),
                },
            });
        }
        return Result{std::move(queuedJobs)};
    } catch (const std::exception& ex) {
        return Result<std::vector<QueuedJobInfo>>::Failure<PgException>(toString(
            "Unexpected exception while getting queued jobs: ", ex.what()));
    }
}

Result<std::vector<ActiveVm>> PgTaskStorage::getActiveVms_(pqxx::transaction_base& txn) noexcept
{
    const auto activeVmsQuery = toString(
        "SELECT id, cpu, ram, cpu_idle, ram_idle FROM scheduler.vms ",
        "WHERE status IN ", pg::asFormattedList(getActiveVmStatuses()), ";");
    try {
        const auto vmsResult = pg::execQuery(activeVmsQuery, txn);

        std::vector<ActiveVm> vms;
        vms.reserve(vmsResult.affected_rows());
        for (const auto& row : vmsResult) {
            vms.push_back(ActiveVm{
                .id = row.at("id").as<VmId>(),
                .totalCapacity = {
                    .cpu = CpuCores(row.at("cpu").as<size_t>()),
                    .ram = MegaBytes(row.at("ram").as<size_t>()),
                },
                .idleCapacity = {
                    .cpu = CpuCores(row.at("cpu_idle").as<size_t>()),
                    .ram = MegaBytes(row.at("ram_idle").as<size_t>()),
                },
            });
        }
        return Result{std::move(vms)};
    } catch (const std::exception& ex) {
        return Result<std::vector<ActiveVm>>::Failure<PgException>(toString(
            "Unexpected exception while getting active vms: ", ex.what()));
    }
}

Result<pg::TransactionHandle> PgTaskStorage::readOnlyTransaction_() noexcept
{
    try {
        auto txn = pool_.readOnlyTransaction();
        return Result{std::move(txn)};
    } catch (const std::exception& ex) {
        return Result<pg::TransactionHandle>::Failure<PgException>(toString(
            "Unexpected exception while creating master read-only transaction: ", ex.what()));
    }
}

Result<std::vector<AllocationPendingVmInfo>> PgTaskStorage::getVmsToAllocate(const size_t maxVmAllocationCount) noexcept
{
    const auto vmsToAllocateQuery = toString(
        "UPDATE scheduler.vms SET status = '", toString(VmStatus::Allocating), "' ",
        "WHERE id IN ("
        "SELECT id FROM scheduler.vms WHERE status = '", toString(VmStatus::PendingAllocation),
        "' LIMIT ", maxVmAllocationCount, " FOR UPDATE SKIP LOCKED) "
        "RETURNING id, cpu, ram;");

    auto vmsResult = execWritableQuery_(vmsToAllocateQuery);
    if (vmsResult.IsSuccess()) {
        std::vector<AllocationPendingVmInfo> vms;
        vms.reserve(vmsResult.ValueRefOrThrow().affected_rows());
        for (const auto& row : vmsResult.ValueRefOrThrow()) {
            vms.push_back(AllocationPendingVmInfo{
                .id = row.at("id").as<VmId>(),
                .capacity = {
                    .cpu = CpuCores(row.at("cpu").as<size_t>()),
                    .ram = MegaBytes(row.at("ram").as<size_t>()),
                }
            });
        }
        return Result{std::move(vms)};
    } else {
        return Result<std::vector<AllocationPendingVmInfo>>::Failure(std::move(vmsResult).ErrorOrThrow());
    }
}

Result<std::vector<TerminationPendingVmInfo>> PgTaskStorage::getVmsToTerminate(
    const size_t maxVmATerminationCount) noexcept
{
    const auto vmsToTerminateQuery = toString(
        "UPDATE scheduler.vms SET status = '", toString(VmStatus::Terminating), "' ",
        "WHERE id IN ("
        "SELECT id FROM scheduler.vms WHERE status = '", toString(VmStatus::PendingTermination),
        "' LIMIT ", maxVmATerminationCount, " FOR UPDATE SKIP LOCKED) "
        "RETURNING id, cloud_vm_id;");

    auto vmsResult = execWritableQuery_(vmsToTerminateQuery);
    if (vmsResult.IsSuccess()) {
        std::vector<TerminationPendingVmInfo> vms;
        vms.reserve(vmsResult.ValueRefOrThrow().affected_rows());
        for (const auto& row : vmsResult.ValueRefOrThrow()) {
            vms.push_back(TerminationPendingVmInfo{
                .id = row.at("id").as<VmId>(),
                .cloudVmId = row.at("cloud_vm_id").as<CloudVmId>(),
            });
        }
        return Result{std::move(vms)};
    } else {
        return Result<std::vector<TerminationPendingVmInfo>>::Failure(std::move(vmsResult).ErrorOrThrow());
    }
}

Result<void> PgTaskStorage::setVmStatus_(const std::vector<VmId>& vmIds, const VmStatus status) noexcept
{
    const auto setVmStatusQuery = toString(
        "UPDATE scheduler.vms SET status = '", toString(status),
        "' WHERE id IN (", joinSeq(vmIds), ");");

    auto result = execWritableQuery_(setVmStatusQuery);
    if (result.IsSuccess()) {
        return Result<void>::Success();
    } else {
        return Result<void>::Failure(std::move(result).ErrorOrThrow());
    }
}

Result<void> PgTaskStorage::returnUnterminatedVms(const std::vector<VmId>& vmIds) noexcept
{
    return setVmStatus_(vmIds, VmStatus::PendingTermination);
}

Result<void> PgTaskStorage::saveVmTerminationResult(const VmId vmId) noexcept
{
    return setVmStatus_({vmId}, VmStatus::Terminated);
}

Result<void> PgTaskStorage::cancelTask(const TaskId taskId) noexcept
{
    const auto cancelTaskQuery = toString(
        "UPDATE scheduler.jobs SET status = ",
        "(CASE WHEN status = '", toString(JobStatus::Queued), "' ",
        "then '", toString(JobStatus::Cancelled), "' "
        "else '", toString(JobStatus::Cancelling), "' end)::scheduler.job_status "
        "WHERE task_id = ", taskId, " AND status NOT IN ",
        pg::asFormattedList(getFinalJobStatuses()), ";");

    auto result = execWritableQuery_(cancelTaskQuery);
    if (result.IsSuccess()) {
        return Result<void>::Success();
    } else {
        return Result<void>::Failure(std::move(result).ErrorOrThrow());
    }
}

Result<void> PgTaskStorage::saveVmAllocationResult(const VmId id, const AllocatedVmInfo& allocatedVmInfo) noexcept
{
    const auto saveVmAllocationResultQuery = toString(
        "UPDATE scheduler.vms SET status = '", toString(VmStatus::Allocated),
        "', cloud_vm_id = '", allocatedVmInfo.id, "', cloud_vm_type = '", allocatedVmInfo.type,
        "' WHERE id = ", id, ";");

    auto result = execWritableQuery_(saveVmAllocationResultQuery);
    if (result.IsSuccess()) {
        return Result<void>::Success();
    } else {
        return Result<void>::Failure(std::move(result).ErrorOrThrow());
    }
}

Result<JobStates> PgTaskStorage::getJobStates(const TaskId taskId) noexcept
{
    const auto getJobStatesQuery = toString(
        "SELECT status, COALESCE(result_url, '') as result_url FROM scheduler.jobs WHERE task_id = ", taskId, ";");

    try {
        auto txn = pool_.readOnlyTransaction();
        auto result = pg::execQuery(getJobStatesQuery, *txn);

        JobStates jobStates;
        jobStates.reserve(result.size());
        std::transform(result.begin(), result.end(), std::back_inserter(jobStates),
                       [](const pqxx::row& r) -> JobState {
                           return {
                               .status = jobStatusFromString(r.at("status").as<std::string>()),
                               .resultUrl = r.at("result_url").as<std::optional<JobResultUrl>>(),
                           };
                       });
        return Result{jobStates};
    } catch (const std::exception& ex) {
        ERROR() << ex.what();
        return Result<JobStates>::Failure<PgException>(toString(
            "Unexpected pg exception: ", ex.what()));
    }
}

Result<void> PgTaskStorage::restartAllocatingVms_(
    const std::string& restartCondition,
    const size_t vmRestartAttemptCount) noexcept
{
    const auto restartAllocatingVmsQuery = toString(
        "UPDATE scheduler.vms "
        "SET status = '", toString(VmStatus::PendingAllocation), "', ",
        "restart_count = restart_count + 1",
        "WHERE status = '", toString(VmStatus::Allocating), "' ",
        "AND restart_count < ", vmRestartAttemptCount, " ",
        "AND " + restartCondition,
        "RETURNING id;"
    );
    auto result = execWritableQuery_(restartAllocatingVmsQuery);
    if (result.IsFailure()) {
        return Result<void>::Failure(std::move(result).ErrorOrThrow());
    }

    const auto reallocatingVmIds = pg::extractIds<VmId>(result.ValueRefOrThrow());
    INFO() << "Moved vms from status " << toString(VmStatus::Allocating)
           << " to " << toString(VmStatus::PendingAllocation)
           << ": [" << joinSeq(reallocatingVmIds) << "]";
    return Result<void>::Success();
}

Result<void> PgTaskStorage::restartStaleAllocatingVms(
    const std::chrono::seconds& allocationTimeLimit,
    const size_t vmRestartAttemptCount) noexcept
{
    const auto restartCondition = toString(
        "NOW() > last_status_update + interval '",
        allocationTimeLimit.count(), " seconds' ");
    return restartAllocatingVms_(restartCondition, vmRestartAttemptCount);
}

namespace {

void restartJobs(
    pqxx::transaction_base& txn,
    const std::vector<VmId>& vmIds,
    const JobStatus initialStatus,
    const size_t jobRestartAttemptCount)
{
    const auto returnJobsToQueuedStatusQuery = toString(
        "UPDATE scheduler.jobs ",
        "SET status = '", toString(JobStatus::Queued), "', ",
        "restart_count = restart_count + 1 ",
        "WHERE status = '", toString(initialStatus), "' ",
        "AND restart_count < ", jobRestartAttemptCount, " ",
        "AND vm_id IN (", joinSeq(vmIds), ")",
        "RETURNING id;"
    );
    const auto failJobsQuery = toString(
        "UPDATE scheduler.jobs SET status = '", toString(JobStatus::InternalError), "' ",
        "WHERE status = '", toString(initialStatus), "' ",
        "AND restart_count >= ", jobRestartAttemptCount, " ",
        "AND vm_id IN (", joinSeq(vmIds), ")",
        "RETURNING id;"
    );

    const auto queuedJobs = pg::execQuery(returnJobsToQueuedStatusQuery, txn);
    const auto queuedJobIds = pg::extractIds<JobId>(queuedJobs);
    INFO() << "Restart jobs: change status " << toString(initialStatus) << " -> "
           << toString(JobStatus::Queued) << ": [" << joinSeq(queuedJobIds) << "]";

    const auto failedJobs = pg::execQuery(failJobsQuery, txn);
    const auto failedJobIds = pg::extractIds<JobId>(failedJobs);
    INFO() << "Restart jobs: change status " << toString(initialStatus) << " -> "
           << toString(JobStatus::InternalError) << ": [" << joinSeq(failedJobIds) << "]";
}

} // anonymous namespace


Result<void> PgTaskStorage::terminateVms_(
    const std::string& terminateCondition,
    const VmStatus initialStatus,
    const VmStatus targetStatus,
    const size_t vmRestartAttemptCount,
    const size_t jobRestartAttemptCount) noexcept
{
    const auto terminateVmsQuery = toString(
        "UPDATE scheduler.vms ",
        "SET status = '", toString(targetStatus), "' ",
        "WHERE status = '", toString(initialStatus), "' ",
        "AND restart_count >= ", vmRestartAttemptCount, " ",
        "AND ", terminateCondition, " ",
        "RETURNING id;"
    );
    try {
        auto txn = pool_.writableTransaction();
        auto terminatedVms = pg::execQuery(terminateVmsQuery, *txn);

        const auto terminatedVmIds = pg::extractIds<VmId>(terminatedVms);
        if (terminatedVmIds.empty()) {
            return Result<void>::Success();
        }
        INFO() << "Moved vms from status " << toString(initialStatus)
               << " to " << toString(targetStatus)
               << ": [" << joinSeq(terminatedVmIds) << "]";

        const auto initialJobStatus = JobStatus::Scheduled;
        restartJobs(*txn, terminatedVmIds, initialJobStatus, jobRestartAttemptCount);

        txn->commit();
        return Result<void>::Success();
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString(
            "Unexpected exception while handling stale allocating VMs: ", ex.what()));
    }
}

Result<void> PgTaskStorage::terminateStaleAllocatingVms(
    const std::chrono::seconds& allocationTimeLimit,
    const size_t vmRestartAttemptCount,
    const size_t jobRestartAttemptCount) noexcept
{
    const std::string terminateCondition = toString(
        "NOW() > last_status_update + interval '", allocationTimeLimit.count(), " seconds'");
    const auto vmInitialStatus = VmStatus::Allocating;
    const auto vmTargetStatus = VmStatus::Terminated;
    return terminateVms_(
        terminateCondition, vmInitialStatus, vmTargetStatus,
        vmRestartAttemptCount, jobRestartAttemptCount);
}

Result<void> PgTaskStorage::rollbackUnallocatedVmsState(
    const std::vector<VmId>& vmIds,
    const size_t vmRestartAttemptCount,
    const size_t jobRestartAttemptCount) noexcept
{
    const auto restartCondition = toString("id IN (", joinSeq(vmIds), ") ");
    const auto restartResult = restartAllocatingVms_(restartCondition, vmRestartAttemptCount);
    if (restartResult.IsFailure()) {
        return restartResult;
    }
    const std::string terminateCondition = toString(
        "id IN (", joinSeq(vmIds), ")");
    const auto vmInitialStatus = VmStatus::Allocating;
    const auto vmTargetStatus = VmStatus::Terminated;
    return terminateVms_(
        terminateCondition, vmInitialStatus, vmTargetStatus,
        vmRestartAttemptCount, jobRestartAttemptCount);
}

Result<void> PgTaskStorage::terminateVmsWithInactiveAgents(
    const std::chrono::seconds& agentInactivityTimeLimit,
    const size_t jobRestartAttemptCount) noexcept
{
    const auto terminateInactiveVmsQuery = toString(
        "UPDATE scheduler.vms "
        "SET status = '", toString(VmStatus::PendingTermination), "' ",
        "WHERE status = '", toString(VmStatus::AgentStarted), "' ",
        "AND NOW() > agent_activity + interval '", agentInactivityTimeLimit.count(), " seconds' ",
        "RETURNING id;"
    );
    try {
        auto txn = pool_.writableTransaction();
        auto pendingTerminationVms = pg::execQuery(terminateInactiveVmsQuery, *txn);

        const auto pendingTerminationVmIds = pg::extractIds<VmId>(pendingTerminationVms);
        if (pendingTerminationVmIds.empty()) {
            return Result<void>::Success();
        }
        INFO() << "Handle VMs with inactive agents: set status "
               << toString(VmStatus::PendingTermination) << " for VMs: ["
               << joinSeq(pendingTerminationVmIds) << "]";

        const auto initialJobStatus = JobStatus::Running;
        restartJobs(*txn, pendingTerminationVmIds, initialJobStatus, jobRestartAttemptCount);

        txn->commit();
        return Result<void>::Success();
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString(
            "Unexpected exception while handling VMs with inactive agents: ", ex.what()));
    }
}

Result<void> PgTaskStorage::terminateVmsWithoutAgents(
    const std::chrono::seconds& agentStartupTimeLimit,
    const size_t jobRestartAttemptCount) noexcept
{
    const std::string terminateCondition = toString(
        "NOW() > last_status_update + interval '", agentStartupTimeLimit.count(), " seconds'");
    const auto vmInitialStatus = VmStatus::Allocated;
    const auto vmTargetStatus = VmStatus::PendingTermination;
    const auto vmRestartAttemptCount = 0;
    return terminateVms_(
        terminateCondition, vmInitialStatus, vmTargetStatus,
        vmRestartAttemptCount, jobRestartAttemptCount);
}

namespace {

std::vector<AssignedJob> pgResultToAssignedJobs(const pqxx::result& result)
{
    std::vector<AssignedJob> assignedJobs;
    assignedJobs.reserve(result.size());
    std::transform(result.begin(), result.end(), std::back_inserter(assignedJobs),
                   [](const pqxx::row& r) -> AssignedJob {
                       return {
                           .id = r.at("id").as<JobId>(),
                           .status = jobStatusFromString(r.at("status").as<std::string>()),
                       };
                   });
    return assignedJobs;
}

} // anonymous namespace


Result<std::vector<AssignedJob>> PgTaskStorage::getAssignedJobs(const VmId vmId) noexcept
{
    const auto updateAgentStartedStatusQuery = toString(
        "UPDATE scheduler.vms "
        "SET status = '", toString(VmStatus::AgentStarted), "' "
        "WHERE id = ", vmId, " "
        "AND status = '", toString(VmStatus::Allocated), "' "
        "RETURNING id;"
    );
    const auto updateAgentActivityQuery = toString(
        "UPDATE scheduler.vms "
        "SET agent_activity = NOW() "
        "WHERE id = ", vmId, ";"
    );
    const auto getAssignedJobsQuery = toString(
        "SELECT id, status FROM scheduler.jobs "
        "WHERE vm_id = ", vmId, " AND "
        "status IN ", pg::asFormattedList(getAssignedJobStatuses()),
        ";");

    try {
        auto txn = pool_.writableTransaction();
        auto updateVmStatusResult = pg::execQuery(updateAgentStartedStatusQuery, *txn);
        if (!updateVmStatusResult.empty()) {
            INFO() << "Agent on VM with id " << vmId << " connected";
        }

        pg::execQuery(updateAgentActivityQuery, *txn);

        auto assignedJobs = pg::execQuery(getAssignedJobsQuery, *txn);
        txn->commit();

        return Result{pgResultToAssignedJobs(assignedJobs)};
    } catch (const std::exception& ex) {
        return Result<std::vector<AssignedJob>>::Failure<PgException>(toString(
            "Unexpected pg exception: ", ex.what()));
    }
}

namespace {

JobToLaunch pgResultToJobToLaunch(const pqxx::row& result)
{
    return {
        .id = result.at("id").as<JobId>(),
        .capacityLimits = {
            .cpu = CpuCores(result.at("cpu").as<size_t>()),
            .ram = MegaBytes(result.at("ram").as<size_t>()),
        },
        .taskSettings = result.at("task_settings").as<TaskSettings>(),
        .imageVersion = result.at("image_version").as<ImageVersion>(),
        .jobOptions = result.at("options").as<ImageVersion>(),
    };
}

} // anonymous namespace

Result<JobToLaunch> PgTaskStorage::getJobToLaunch(const VmId vmId, const JobId jobId) noexcept
{
    const auto getJobQuery = toString(
        "SELECT jobs.id, jobs.cpu, jobs.ram, jobs.options, tasks.image_version, tasks.settings as task_settings "
        "FROM scheduler.jobs "
        "JOIN scheduler.tasks ON scheduler.jobs.task_id = scheduler.tasks.id "
        "WHERE vm_id = ", vmId, " AND "
        "jobs.id = ", jobId,
        ";");

    try {
        auto txn = pool_.readOnlyTransaction();
        auto result = pg::execQuery(getJobQuery, *txn);
        if (result.empty()) {
            return Result<JobToLaunch>::Failure<JobNotFoundException>(toString(
                "No job with id '", jobId, "' assigned for VM with id '", vmId, "'."));
        }

        return Result{pgResultToJobToLaunch(result[0])};
    } catch (const std::exception& ex) {
        return Result<JobToLaunch>::Failure<PgException>(toString(
            "Unexpected pg exception: ", ex.what()));
    }
}

Result<void> PgTaskStorage::updateJobState(
    const VmId vmId, const JobId jobId, const JobState& jobState) noexcept
{
    try {
        auto txn = pool_.writableTransaction();
        const auto lockGetStatusQuery = toString(
            "SELECT status FROM scheduler.jobs "
            "WHERE vm_id = ", vmId, " AND "
            "jobs.id = ", jobId, " "
            "FOR UPDATE;");

        auto statusResult = pg::execQuery(lockGetStatusQuery, *txn);
        if (statusResult.empty()) {
            return Result<void>::Failure<JobNotFoundException>(toString(
                "No job with id '", jobId, "' assigned for VM with id '", vmId, "'."));
        }
        const auto jobStatus = jobStatusFromString(statusResult[0].at("status").as<std::string>());
        if (jobStatus == JobStatus::Cancelling && jobState.status != JobStatus::Cancelled) {
            return Result<void>::Failure<JobCancelledException>(toString(
                "Job with id '", jobId, "' assigned for VM with id '", vmId, "' was cancelled."));
        }

        const auto& finalJobStatuses = getFinalJobStatuses();
        const auto updateStatement
            = finalJobStatuses.contains(jobState.status)
                  ? "finished = NOW() "
                  : "started = COALESCE(started, NOW()) ";
        const auto updateJobQuery = toString(
            "UPDATE scheduler.jobs "
            "SET result_url = ", txn->quote(jobState.resultUrl), ", "
            "status = '", toString(jobState.status), "', ",
            updateStatement,
            "WHERE vm_id = ", vmId, " AND "
            "jobs.id = ", jobId, " AND "
            "status NOT IN ", pg::asFormattedList(finalJobStatuses),
            ";");

        pg::execQuery(updateJobQuery, *txn);
        if (finalJobStatuses.contains(jobState.status)) {
            const auto updateVmIdleCapacity = toString(
                "UPDATE scheduler.vms AS v "
                    "SET cpu_idle = cpu_idle + j.cpu, "
                    "ram_idle = ram_idle + j.ram "
                "FROM scheduler.jobs AS j "
                "WHERE v.id = j.vm_id "
                    "AND j.id = ", jobId, " "
                    "AND j.vm_id = ", vmId, ";"
            );
            pg::execQuery(updateVmIdleCapacity, *txn);
        }
        txn->commit();
        return Result<void>::Success();
    } catch (const std::exception& ex) {
        return Result<void>::Failure<PgException>(toString(
            "Unexpected pg exception while updating job state: ", ex.what()));
    }
}

Result<AllocatedVmInfos> PgTaskStorage::getAllocatedVms() noexcept
{
    const auto allocatedVmsQuery = toString(
        "SELECT cloud_vm_id, cloud_vm_type FROM scheduler.vms "
        "WHERE status IN ", pg::asFormattedList(getActiveVmStatuses()), ";");
    try {
        auto txn = pool_.readOnlyTransaction();
        const auto result = pg::execQuery(allocatedVmsQuery, *txn);

        AllocatedVmInfos vms;
        vms.reserve(result.size());

        std::transform(result.begin(), result.end(), std::inserter(vms, vms.begin()),
                       [](const pqxx::row& row) -> AllocatedVmInfo {
                           return {
                               .id = row.at("cloud_vm_id").as<std::string>(),
                               .type = row.at("cloud_vm_type").as<std::string>(),
                           };
                       });
        return Result{std::move(vms)};
    } catch (const std::exception& ex) {
        return Result<AllocatedVmInfos>::Failure<PgException>(toString(
            "Unexpected pg exception while receiving allocated VMs: ", ex.what()));
    }
}


Result<void> PgTaskStorage::cancelTimedOutJobs() noexcept
{
    const auto cancelTimedOutJobsQuery = toString(
        "UPDATE scheduler.jobs "
        "SET status = '", toString(JobStatus::Cancelled), "' "
        "WHERE created + estimation < NOW() "
        "AND status NOT IN ", pg::asFormattedList(getFinalJobStatuses()), " "
        "RETURNING id;"
    );

    auto result = execWritableQuery_(cancelTimedOutJobsQuery);
    if (result.IsSuccess()) {
        auto cancelledJobIds = pg::extractIds<VmId>(result.ValueRefOrThrow());
        INFO() << "Moved jobs to status " << toString(JobStatus::Cancelled)
               << ": [" << joinSeq(cancelledJobIds) << "]";
        return Result<void>::Success();
    } else {
        return Result<void>::Failure(std::move(result).ErrorOrThrow());
    }
}

} // namespace vm_scheduler
