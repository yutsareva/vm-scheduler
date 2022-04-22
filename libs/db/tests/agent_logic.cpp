#include "libs/db/include/pg_task_storage.h"
#include "libs/db/impl/test_utils.h"
#include "libs/postgres/include/pg_pool.h"

#include <libs/task_storage/include/task_storage.h>
#include <libs/scheduler/include/scheduler.h>
#include <libs/common/include/errors.h>
#include <libs/common/include/stringify.h>
#include <libs/state/include/errors.h>
#include <libs/state/include/task.h>

#include <libs/postgres/include/helpers.h>

#include <gtest/gtest.h>

#include <memory>


using namespace vm_scheduler;
using namespace std::chrono_literals;
namespace t = vm_scheduler::testing;

namespace {

JobState getJobState(const JobStatus status) {
    return {
        .status = status,
        .resultUrl
        = (status == JobStatus::Completed)
              ? std::optional<std::string>("result url")
              : std::nullopt,
    };
}

}


TEST(AgentLogic, fullScenario)
{
    auto pool = pg::createPool();
    const auto vmId = t::insertVm(pool, VmStatus::Allocated);

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

    const auto assignedJobs = pgTaskStorage.getAssignedJobs(vmId);
    EXPECT_TRUE(assignedJobs.IsSuccess());
    EXPECT_EQ(assignedJobs.ValueRefOrThrow().size(), 3u);

    const auto jobToLaunch = pgTaskStorage.getJobToLaunch(vmId, assignedJobs.ValueRefOrThrow()[0].id);
    EXPECT_TRUE(jobToLaunch.IsSuccess());
    EXPECT_EQ(jobToLaunch.ValueRefOrThrow().id, assignedJobs.ValueRefOrThrow()[0].id);
    EXPECT_EQ(jobToLaunch.ValueRefOrThrow().capacityLimits, taskParameters.requiredCapacity);
    EXPECT_EQ(jobToLaunch.ValueRefOrThrow().taskSettings, taskParameters.settings);
    EXPECT_EQ(jobToLaunch.ValueRefOrThrow().imageVersion, taskParameters.imageVersion);
    EXPECT_EQ(jobToLaunch.ValueRefOrThrow().jobOptions, taskParameters.jobOptions[0]);

    const auto updateResult = pgTaskStorage.updateJobState(
        vmId, assignedJobs.ValueRefOrThrow()[0].id, getJobState(JobStatus::Running));
    EXPECT_TRUE(updateResult.IsSuccess());

    const auto jobInfoQuery = toString(
        "SELECT started, finished, status, result_url FROM scheduler.jobs WHERE id = ",
        assignedJobs.ValueRefOrThrow()[0].id, ";");
    const auto vmInfoQuery = toString(
        "SELECT cpu, cpu_idle, ram, ram_idle FROM scheduler.vms WHERE id = ", vmId, ";");

    auto readTxn = pool.readOnlyTransaction();

    {
        const auto jobInfo = pg::execQuery(jobInfoQuery, *readTxn);
        EXPECT_EQ(jobStatusFromString(jobInfo[0].at("status").as<std::string>()), JobStatus::Running);
        EXPECT_TRUE(jobInfo[0].at("started").as<std::optional<std::string>>());
        EXPECT_TRUE(!jobInfo[0].at("finished").as<std::optional<std::string>>());
        EXPECT_TRUE(!jobInfo[0].at("result_url").as<std::optional<std::string>>());

        const auto vmInfo = pg::execQuery(vmInfoQuery, *readTxn);
        EXPECT_EQ(vmInfo[0].at("cpu").as<std::optional<size_t>>(), 2u);
        EXPECT_EQ(vmInfo[0].at("cpu_idle").as<std::optional<size_t>>(), 1u);
        EXPECT_EQ(vmInfo[0].at("ram").as<std::optional<size_t>>(), 2048u);
        EXPECT_EQ(vmInfo[0].at("ram_idle").as<std::optional<size_t>>(), 1024u);
    }

    const auto finishJobResult = pgTaskStorage.updateJobState(
        vmId, assignedJobs.ValueRefOrThrow()[0].id, getJobState(JobStatus::Completed));
    EXPECT_TRUE(finishJobResult.IsSuccess());
    {
        const auto jobInfo = pg::execQuery(jobInfoQuery, *readTxn);
        EXPECT_EQ(jobStatusFromString(jobInfo[0].at("status").as<std::string>()), JobStatus::Completed);
        EXPECT_TRUE(jobInfo[0].at("started").as<std::optional<std::string>>());
        EXPECT_TRUE(jobInfo[0].at("finished").as<std::optional<std::string>>());
        EXPECT_TRUE(jobInfo[0].at("result_url").as<std::optional<std::string>>());

        const auto vmInfo = pg::execQuery(vmInfoQuery, *readTxn);
        EXPECT_EQ(vmInfo[0].at("cpu").as<std::optional<size_t>>(), 2u);
        EXPECT_EQ(vmInfo[0].at("cpu_idle").as<std::optional<size_t>>(), 2u);
        EXPECT_EQ(vmInfo[0].at("ram").as<std::optional<size_t>>(), 2048u);
        EXPECT_EQ(vmInfo[0].at("ram_idle").as<std::optional<size_t>>(), 2048u);
    }
}

TEST(AgentLogic, jobNotFound)
{
    auto pool = pg::createPool();
    const auto vmId = t::insertVm(pool, VmStatus::AgentStarted);

    PgTaskStorage pgTaskStorage(std::move(pool));

    const auto assignedJobs = pgTaskStorage.getAssignedJobs(vmId);
    EXPECT_TRUE(assignedJobs.IsSuccess());
    EXPECT_EQ(assignedJobs.ValueRefOrThrow().size(), 0u);

    const JobId nonExistingJobId = 13;
    const auto finishJobResult = pgTaskStorage.updateJobState(
        vmId, nonExistingJobId, getJobState(JobStatus::Completed));
    EXPECT_TRUE(finishJobResult.IsFailure());
    EXPECT_TRUE(finishJobResult.holdsErrorType<JobNotFoundException>());
}

TEST(AgentLogic, vmNotFound)
{
    PgTaskStorage pgTaskStorage(pg::createPool());

    const VmId nonExistingVmId = 42;
    const auto assignedJobs = pgTaskStorage.getAssignedJobs(nonExistingVmId);
    EXPECT_TRUE(assignedJobs.IsSuccess());
    EXPECT_EQ(assignedJobs.ValueRefOrThrow().size(), 0u);

    const JobId nonExistingJobId = 13;
    const auto finishJobResult = pgTaskStorage.updateJobState(
        nonExistingVmId, nonExistingJobId, getJobState(JobStatus::Completed));
    EXPECT_TRUE(finishJobResult.IsFailure());
    EXPECT_TRUE(finishJobResult.holdsErrorType<JobNotFoundException>());
}
