#include <libs/allocator/include/cloud_client_mock.h>
#include <libs/db/include/pg_task_storage.h>
#include <libs/postgres/include/helpers.h>
#include <libs/server/impl/test_utils.h>
#include <libs/task_registry/include/task_registry.h>
#include <libs/common/include/stringify.h>

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
    setenv("PG_HOSTS", "host.docker.internal", true);
    setenv("PG_POOL_SIZE", "10", true);

    setenv("VMS_ZK_ADDRESS", "zk://3.137.201.107:2181,3.131.159.162:2181,3.17.63.120:2181/", true);
}

void checkJobStatuses(const proto::TaskExecutionResult& taskResult, const proto::JobStatus& status)
{
    EXPECT_EQ(taskResult.status(), proto::TaskStatus::TASK_COMPLETED);
    EXPECT_EQ(taskResult.job_results().size(), 2);
    EXPECT_EQ(taskResult.job_results()[0].status(), status);
    EXPECT_EQ(taskResult.job_results()[1].status(), status);
}

} // namespace

TEST(ZooKeeper, ok)
{
    setupEnv();

    const Config config = {
        .allocationInterval = 1s,
        .scheduleInterval = 1s,
        .detectFailuresInterval = 1s,
        .mode = SchedulerMode::FullScheduler,
        .useZkDistLock = true,
    };
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();
    EXPECT_CALL(*cloudClientMock, allocate(_, _))
        .WillRepeatedly(Return(Result<AllocatedVmInfo>::Failure<RuntimeException>(
            "Unexpected exception while requesting a VM")));
    EXPECT_CALL(*cloudClientMock, getAllAllocatedVms())
        .WillRepeatedly(Return(Result{AllocatedVmInfos{}}));
    EXPECT_CALL(*cloudClientMock, getPossibleSlots())
        .WillRepeatedly(Return(std::vector<SlotCapacity>{
            SlotCapacity{
                .cpu = 1_cores,
                .ram = 1024_MB,
            },
        }));
    auto taskRegistry = std::make_unique<TaskRegistry>(
        config,
        std::make_unique<PgTaskStorage>(pg::createPool()),
        std::move(cloudClientMock));

    const auto protoTaskAdditionResult = t::addTask();
    const auto taskResult =
        t::waitTaskForComplete(protoTaskAdditionResult.task_id(), 30s);
    checkJobStatuses(taskResult, proto::JobStatus::JOB_FAILED);
}
