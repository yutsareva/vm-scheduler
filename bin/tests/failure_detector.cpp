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
}

void checkJobStatuses(const proto::TaskExecutionResult& taskResult, const proto::JobStatus& status)
{
    EXPECT_EQ(taskResult.status(), proto::TaskStatus::TASK_COMPLETED);
    EXPECT_EQ(taskResult.job_results().size(), 2);
    EXPECT_FALSE(taskResult.job_results()[0].has_result_url());
    EXPECT_EQ(taskResult.job_results()[0].status(), status);
    EXPECT_FALSE(taskResult.job_results()[1].has_result_url());
    EXPECT_EQ(taskResult.job_results()[1].status(), status);
}

} // namespace

TEST(fail_tasks, non_working_allocator)
{
    setupEnv();

    const Config config = {
        .allocationInterval = 1s,
        .scheduleInterval = 1s,
        .detectFailuresInterval = 1s,
    };
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();
    EXPECT_CALL(*cloudClientMock, allocate(_, _))
        .WillRepeatedly(Return(Result<AllocatedVmInfo>::Failure<RuntimeException>(
            "Unexpected exception while requesting a VM")));
    EXPECT_CALL(*cloudClientMock, getAllAllocatedVms())
        .WillRepeatedly(Return(Result{AllocatedVmInfos{}}));
    EXPECT_CALL(*cloudClientMock, getPossibleSlots())
        .WillOnce(Return(std::vector<SlotCapacity>{
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

TEST(fail_tasks, non_working_agent)
{
    setupEnv();

    const Config config = {
        .allocationInterval = 1s,
        .scheduleInterval = 1s,
        .detectFailuresInterval = 1s,
    };
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();
    const auto allocatedVmInfo = AllocatedVmInfo{
        .id = "cloud vm id",
        .type = "cloud vm type",
    };
    EXPECT_CALL(*cloudClientMock, allocate(_, _))
        .WillRepeatedly(Return(Result{allocatedVmInfo}));
    EXPECT_CALL(*cloudClientMock, terminate(_))
        .WillRepeatedly(Return(Result<void>::Success()));
    EXPECT_CALL(*cloudClientMock, getAllAllocatedVms())
        .WillRepeatedly(Return(Result{AllocatedVmInfos{}}));
    EXPECT_CALL(*cloudClientMock, getPossibleSlots())
        .WillOnce(Return(std::vector<SlotCapacity>{
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

TEST(TerminateUntracked, simple)
{
    setupEnv();

    const Config config = {
        .allocationInterval = 1s,
        .scheduleInterval = 1s,
        .detectFailuresInterval = 1s,
    };
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();
    const auto allocatedVmInfo = AllocatedVmInfo{
        .id = "cloud vm id",
        .type = "cloud vm type",
    };
    EXPECT_CALL(*cloudClientMock, getAllAllocatedVms())
        .WillOnce(Return(Result{AllocatedVmInfos{allocatedVmInfo}}));
    EXPECT_CALL(*cloudClientMock, terminate(_))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(*cloudClientMock, getPossibleSlots())
        .WillOnce(Return(std::vector<SlotCapacity>{
            SlotCapacity{
                .cpu = 1_cores,
                .ram = 1024_MB,
            },
        }));

    auto taskRegistry = std::make_unique<TaskRegistry>(
        config,
        std::make_unique<PgTaskStorage>(pg::createPool()),
        std::move(cloudClientMock));

    std::this_thread::sleep_for(0.5s);
}


TEST(CancelJobs, timedOutJobs)
{
    setupEnv();

    const Config config = {
        .allocationInterval = 10s,
        .scheduleInterval = 1s,
        .detectFailuresInterval = 1s,
    };
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();
    EXPECT_CALL(*cloudClientMock, allocate(_, _))
        .WillRepeatedly(Return(Result{AllocatedVmInfo{}}));
    EXPECT_CALL(*cloudClientMock, terminate(_))
        .WillRepeatedly(Return(Result<void>::Success()));
    EXPECT_CALL(*cloudClientMock, getAllAllocatedVms())
        .WillRepeatedly(Return(Result{AllocatedVmInfos{}}));
    EXPECT_CALL(*cloudClientMock, getPossibleSlots())
        .WillOnce(Return(std::vector<SlotCapacity>{
            SlotCapacity{
                .cpu = 1_cores,
                .ram = 1024_MB,
            },
            SlotCapacity{
                .cpu = 4_cores,
                .ram = 8192_MB,
            },
        }));

    auto taskRegistry = std::make_unique<TaskRegistry>(
        config,
        std::make_unique<PgTaskStorage>(pg::createPool()),
        std::move(cloudClientMock));

    auto protoTask = t::generateProtoTask();
    protoTask.mutable_limits()->set_execution_s(5);

    const auto protoTaskAdditionResult = t::addTask(protoTask);

    std::this_thread::sleep_for(3s);

    auto pool = createPool(postgres);
    const auto getVms = toString(
        "SELECT cpu, ram, cpu_idle, ram_idle, status FROM scheduler.vms;");
    {
        auto txn = pool.masterReadOnlyTransaction();
        const auto vmsResult = pg::execQuery(getVms, *txn);

        EXPECT_EQ(vmsResult.size(), 1u);
        EXPECT_EQ(vmsResult[0].at("cpu").as<size_t>(), 4u);
        EXPECT_EQ(vmsResult[0].at("cpu_idle").as<size_t>(), 0u);
        EXPECT_EQ(vmsResult[0].at("ram").as<size_t>(), 8192u);
        EXPECT_EQ(vmsResult[0].at("ram_idle").as<size_t>(), 6192u);
    }

    const auto taskResult = t::waitTaskForComplete(protoTaskAdditionResult.task_id(), 30s);
    checkJobStatuses(taskResult, proto::JobStatus::JOB_CANCELLED);

    {
        auto txn = pool.masterReadOnlyTransaction();
        const auto vmsResult = pg::execQuery(getVms, *txn);

        EXPECT_EQ(vmsResult.size(), 1u);
        EXPECT_EQ(vmsResult[0].at("cpu").as<size_t>(), 4u);
        EXPECT_EQ(vmsResult[0].at("cpu_idle").as<size_t>(), 4u);
        EXPECT_EQ(vmsResult[0].at("ram").as<size_t>(), 8192u);
        EXPECT_EQ(vmsResult[0].at("ram_idle").as<size_t>(), 8192u);
    }
}
