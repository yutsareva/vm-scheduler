#include <libs/allocator/include/cloud_client_mock.h>
#include <libs/db/include/pg_task_storage.h>
#include <libs/postgres/include/helpers.h>
#include <libs/server/impl/test_utils.h>
#include <libs/task_registry/include/task_registry.h>

#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <thread>

using namespace vm_scheduler;
namespace t = vm_scheduler::testing;
using namespace ::testing;
using namespace std::chrono_literals;

namespace {
void setupEnv()
{
    setenv("VMS_SCHEDULING_INTERVAL_S", "0", true);
    setenv("VMS_ALLOCATION_TIME_LIMIT_S", "1", true);
    setenv("VMS_AGENT_STARTUP_TIME_LIMIT_S", "1", true);
    setenv("VMS_MAX_VM_ALLOCATION_COUNT", "2", true);

    setenv("VMS_VM_RESTART_ATTEMPT_COUNT", "1", true);
    setenv("VMS_JOB_RESTART_ATTEMPT_COUNT", "1", true);

    setenv("PG_USER", "postgres", true);
    setenv("PG_PASSWORD", "postgres", true);
    setenv("PG_DB_NAME", "postgres", true);
    setenv("PG_HOSTS", "localhost", true);
    setenv("PG_POOL_SIZE", "10", true);
}

void checkAllJobsFailed(const proto::TaskExecutionResult& taskResult)
{
    EXPECT_EQ(taskResult.status(), proto::TaskStatus::TASK_COMPLETED);
    EXPECT_EQ(taskResult.job_results().size(), 2);
    EXPECT_FALSE(taskResult.job_results()[0].has_result_url());
    EXPECT_EQ(taskResult.job_results()[0].status(), proto::JobStatus::JOB_FAILED);
    EXPECT_FALSE(taskResult.job_results()[1].has_result_url());
    EXPECT_EQ(taskResult.job_results()[1].status(), proto::JobStatus::JOB_FAILED);
}
} // namespace

TEST(fail_tasks, non_working_allocator)
{
    setupEnv();

    const Config config = {
        .allocationInterval = 1s,
        .schduleInterval = 1s,
        .detectFailuresInterval = 1s,
    };
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();
    EXPECT_CALL(*cloudClientMock, allocate(_))
        .WillRepeatedly(Return(Result<AllocatedVmInfo>::Failure<RuntimeException>(
            "Unexpected exception while requesting a VM")));
    auto taskRegistry = std::make_unique<TaskRegistry>(
        config,
        std::make_unique<PgTaskStorage>(pg::createPool()),
        std::move(cloudClientMock));

    const auto protoTaskAdditionResult = t::addTask();
    const auto taskResult =
        t::waitTaskForComplete(protoTaskAdditionResult.task_id(), 30s);
    checkAllJobsFailed(taskResult);
}

TEST(fail_tasks, non_working_agent)
{
    setupEnv();

    const Config config = {
        .allocationInterval = 1s,
        .schduleInterval = 1s,
        .detectFailuresInterval = 1s,
    };
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();
    const auto allocatedVmInfo = AllocatedVmInfo{
        .id = "cloud vm id",
        .type = "cloud vm type",
    };
    EXPECT_CALL(*cloudClientMock, allocate(_))
        .WillRepeatedly(Return(Result{allocatedVmInfo}));
    EXPECT_CALL(*cloudClientMock, terminate(_))
        .WillRepeatedly(Return(Result<void>::Success()));

    auto taskRegistry = std::make_unique<TaskRegistry>(
        config,
        std::make_unique<PgTaskStorage>(pg::createPool()),
        std::move(cloudClientMock));

    const auto protoTaskAdditionResult = t::addTask();
    const auto taskResult =
        t::waitTaskForComplete(protoTaskAdditionResult.task_id(), 30s);
    checkAllJobsFailed(taskResult);
}
