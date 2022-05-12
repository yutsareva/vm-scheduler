#include "libs/db/include/pg_task_storage.h"
#include "libs/db/impl/test_utils.h"
#include "libs/db/tests/helpers.h"

#include <libs/postgres/include/pg_pool.h>
#include <libs/postgres/include/helpers.h>
#include <libs/task_storage/include/task_storage.h>
#include <libs/scheduler/include/scheduler.h>
#include <libs/common/include/stringify.h>
#include <libs/state/include/task.h>

#include <gmock/gmock.h>

#include <memory>

using namespace vm_scheduler;
using namespace std::chrono_literals;
namespace t = vm_scheduler::testing;

namespace {

void checkStatuses(
    pg::PgPool& pool,
    const VmId vmId,
    const std::vector<JobId>& jobIds,
    const VmStatus expectedVmStatus,
    const JobStatus expectedJobStatus,
    const size_t expectedVmRestartCount,
    const size_t expectedJobRestartCount)
{
    const auto vmQuery = toString("SELECT status, restart_count FROM scheduler.vms WHERE id = ", vmId, ";");
    const auto jobQuery = toString(
        "SELECT status, restart_count FROM scheduler.jobs WHERE id IN (",
        joinSeq(jobIds), ");");
    auto readTxn = pool.readOnlyTransaction();
    const auto vmResult = pg::execQuery(vmQuery, *readTxn);
    EXPECT_EQ(vmResult[0].at("restart_count").as<size_t>(), expectedVmRestartCount);
    EXPECT_EQ(
        vmStatusFromString(vmResult[0].at("status").as<std::string>()),expectedVmStatus);

    const auto jobsResult = pg::execQuery(jobQuery, *readTxn);
    EXPECT_EQ(jobsResult.size(), 3u);
    for (const auto jobResult : jobsResult) {
        EXPECT_EQ(jobResult.at("restart_count").as<size_t>(), expectedJobRestartCount);
        EXPECT_EQ(
            jobStatusFromString(jobResult.at("status").as<std::string>()), expectedJobStatus);
    }
}

} // anonymous namespace

TEST(FailureDetector, restartStaleAllocatingVms)
{
    t::setupEnv();
    auto pool = pg::createPool();
    const auto vmId = t::insertVm(pool, VmStatus::Allocating);

    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto vmQuery = toString("SELECT status, restart_count FROM scheduler.vms WHERE id = ", vmId, ";");
    auto readTxn = pool.readOnlyTransaction();
    {
        const auto result = pg::execQuery(vmQuery, *readTxn);
        EXPECT_EQ(result[0].at("restart_count").as<size_t>(), 0u);
        EXPECT_EQ(
            vmStatusFromString(result[0].at("status").as<std::string>()), VmStatus::Allocating);
    }

    const auto restartStaleAllocatingVmsResult = pgTaskStorage.restartStaleAllocatingVms(0s, 1);
    EXPECT_TRUE(restartStaleAllocatingVmsResult.IsSuccess());

    {
        const auto result = pg::execQuery(vmQuery, *readTxn);
        EXPECT_EQ(result[0].at("restart_count").as<size_t>(), 1u);
        EXPECT_EQ(
            vmStatusFromString(result[0].at("status").as<std::string>()), VmStatus::PendingAllocation);
    }
}

TEST(FailureDetector, terminateStaleAllocatingVms)
{
    t::setupEnv();
    auto pool = pg::createPool();
    const auto vmId = t::insertVm(pool, VmStatus::Allocating);

    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto taskParameters = t::getThreeJobTaskParameters();
    const auto jobsResult = pgTaskStorage.addTask(taskParameters);
    EXPECT_TRUE(jobsResult.IsSuccess());

    auto writeTxn = pool.writableTransaction();
    const auto updateJobsQuery = toString(
        "UPDATE scheduler.jobs ",
        "SET vm_id = ", vmId, ", status = '", toString(JobStatus::Scheduled), "' ",
        "WHERE id IN (", joinSeq(jobsResult.ValueRefOrThrow().jobIds), ");");
    pg::execQuery(updateJobsQuery, *writeTxn);
    writeTxn->commit();

    checkStatuses(
        pool, vmId, jobsResult.ValueRefOrThrow().jobIds,
        VmStatus::Allocating, JobStatus::Scheduled, 0, 0);

    const auto terminateStaleAllocatingVmsResult = pgTaskStorage.terminateStaleAllocatingVms(0s, 0, 1);
    EXPECT_TRUE(terminateStaleAllocatingVmsResult.IsSuccess());

    checkStatuses(
        pool, vmId, jobsResult.ValueRefOrThrow().jobIds,
        VmStatus::Terminated, JobStatus::Queued, 0, 1);
}

TEST(FailureDetector, terminateVmsWithInactiveAgents)
{
    t::setupEnv();
    auto pool = pg::createPool();
    const auto vmId = t::insertVm(pool, VmStatus::AgentStarted);

    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto taskParameters = t::getThreeJobTaskParameters();
    const auto jobsResult = pgTaskStorage.addTask(taskParameters);
    EXPECT_TRUE(jobsResult.IsSuccess());

    auto writeTxn = pool.writableTransaction();
    const auto updateJobsQuery = toString(
        "UPDATE scheduler.jobs ",
        "SET vm_id = ", vmId, ", status = '", toString(JobStatus::Running), "' ",
        "WHERE id IN (", joinSeq(jobsResult.ValueRefOrThrow().jobIds), ");");
    pg::execQuery(updateJobsQuery, *writeTxn);
    const auto updateVmQuery = toString(
        "UPDATE scheduler.vms ",
        "SET agent_activity = NOW() ",
        "WHERE id = ", vmId, ";");
    pg::execQuery(updateVmQuery, *writeTxn);
    writeTxn->commit();

    checkStatuses(
        pool, vmId, jobsResult.ValueRefOrThrow().jobIds,
        VmStatus::AgentStarted, JobStatus::Running, 0, 0);

    const auto terminateVmsWithInactiveAgentsResult = pgTaskStorage.terminateVmsWithInactiveAgents(0s, 1);
    EXPECT_TRUE(terminateVmsWithInactiveAgentsResult.IsSuccess());
    checkStatuses(
        pool, vmId, jobsResult.ValueRefOrThrow().jobIds,
        VmStatus::PendingTermination, JobStatus::Queued, 0, 1);
}
