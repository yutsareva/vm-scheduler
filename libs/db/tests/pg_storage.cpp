#include "libs/db/include/pg_task_storage.h"
#include "libs/db/impl/test_utils.h"
#include "libs/db/tests/helpers.h"

#include <libs/task_storage/include/task_storage.h>
#include <libs/scheduler/include/scheduler.h>
#include <libs/common/include/stringify.h>
#include <libs/state/include/task.h>

#include <libs/postgres/include/helpers.h>

#include <gmock/gmock.h>

#include <memory>


using namespace vm_scheduler;
using namespace std::chrono_literals;
namespace t = vm_scheduler::testing;


TEST(StartScheduling, exclusiveLock)
{
    t::setupEnv();
    PgTaskStorage pgTaskStorage(pg::createPool());

    auto pool = pg::createPool();
    auto txn = pool.writableTransaction();
    const auto lock = "LOCK TABLE scheduler.plan IN ROW EXCLUSIVE MODE;";
    pg::execQuery(lock, *txn);

    const auto planIdResult = pgTaskStorage.startScheduling("backendId", 2s);
    EXPECT_TRUE(planIdResult.IsFailure());
}

TEST(StartScheduling, consecutiveStartScheduling)
{
    t::setupEnv();
    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto firstPlanIdResult = pgTaskStorage.startScheduling("backendId", 2s);
    EXPECT_TRUE(firstPlanIdResult.IsSuccess());

    const auto secondPlanIdResult = pgTaskStorage.startScheduling("backendId", 2s);
    EXPECT_TRUE(secondPlanIdResult.IsFailure());

    const auto thirdPlanIdResult = pgTaskStorage.startScheduling("backendId", 0s);
    EXPECT_TRUE(thirdPlanIdResult.IsSuccess());
}

TEST(GetCurrentState, empty)
{
    t::setupEnv();
    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto currentStateResult = pgTaskStorage.getCurrentState();
    EXPECT_TRUE(currentStateResult.IsSuccess());

    const State emptyState;
    EXPECT_EQ(currentStateResult.ValueRefOrThrow(), emptyState);
}

TEST(CommitPlanChange, fulScenario)
{
    t::setupEnv();
    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto taskParameters = t::getOneJobTaskParameters();
    const auto jobIdResult = pgTaskStorage.addTask(taskParameters);
    EXPECT_TRUE(jobIdResult.IsSuccess());

    const auto planIdResult = pgTaskStorage.startScheduling("backendId", 2s);
    EXPECT_TRUE(planIdResult.IsSuccess());

    const auto stateChange = StateChange{
        .jobToVm = {
            {
                1,
                DesiredSlotId(1),
            },
        },
        .desiredSlotMap = {
            {
                DesiredSlotId(1),
                DesiredSlot{
                    .total = SlotCapacity{
                        .cpu = 3_cores,
                        .ram = 4096_MB,
                    },
                    .idle = SlotCapacity{
                        .cpu = 1_cores,
                        .ram = 1024_MB,
                    },
                },
            },
        },
        .updatedIdleCapacities = { },
        .vmsToTerminate = { },
    };

    auto commitResult = pgTaskStorage.commitPlanChange(stateChange, planIdResult.ValueRefOrThrow());
    EXPECT_TRUE(commitResult.IsSuccess());

    const auto currentStateResult = pgTaskStorage.getCurrentState();
    EXPECT_TRUE(currentStateResult.IsSuccess());

    const auto expectedState = State{
        .queuedJobs = { },
        .vms = {
            ActiveVm{
                .id = 1,
                .totalCapacity = SlotCapacity{
                    .cpu = 3_cores,
                    .ram = 4096_MB,
                },
                .idleCapacity = SlotCapacity{
                    .cpu = 1_cores,
                    .ram = 1024_MB,
                },
            },
        },
    };
    EXPECT_EQ(expectedState, currentStateResult.ValueRefOrThrow());

    auto pool = pg::createPool();
    auto txn = pool.writableTransaction();
    const auto lock = toString("SELECT status, cpu, ram FROM scheduler.jobs WHERE id = ",
                               jobIdResult.ValueRefOrThrow().jobIds[0], ";");
    const auto result = pg::execQuery(lock, *txn);
    EXPECT_EQ(jobStatusFromString(result[0].at("status").as<std::string>()), JobStatus::Scheduled);
    EXPECT_EQ(result[0].at("cpu").as<size_t>(), 1u);
    EXPECT_EQ(result[0].at("ram").as<size_t>(), 1024u);
}


TEST(Allocation, allocate)
{
    t::setupEnv();
    auto pool = pg::createPool();
    const auto vmId = t::insertVm(pool, VmStatus::PendingAllocation);

    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto vmsToAllocateResult = pgTaskStorage.getVmsToAllocate(5);
    EXPECT_TRUE(vmsToAllocateResult.IsSuccess());
    const auto& vmsToAllocate = vmsToAllocateResult.ValueRefOrThrow();
    const std::vector<AllocationPendingVmInfo> expectedVmInfos = {
        AllocationPendingVmInfo{
            .id = vmId,
            .capacity = {
                .cpu = 2_cores,
                .ram = 2048_MB,
            },
        },
    };
    EXPECT_EQ(vmsToAllocate, expectedVmInfos);

    const auto vmStatusQuery = toString("SELECT status FROM scheduler.vms WHERE id = ", vmId, ";");
    auto readTxn = pool.readOnlyTransaction();
    const auto firstVmStatusResult = pg::execQuery(vmStatusQuery, *readTxn);
    EXPECT_EQ(vmStatusFromString(firstVmStatusResult[0].at("status").as<std::string>()), VmStatus::Allocating);

    const auto allocatedVmInfo = AllocatedVmInfo{
        .id = "cloud vm id",
        .type = "cloud vm type",
    };
    const auto saveAllocationResult = pgTaskStorage.saveVmAllocationResult(vmId, allocatedVmInfo);
    EXPECT_TRUE(saveAllocationResult.IsSuccess());

    const auto secondVmStatusResult = pg::execQuery(vmStatusQuery, *readTxn);
    EXPECT_EQ(vmStatusFromString(secondVmStatusResult[0].at("status").as<std::string>()), VmStatus::Allocated);
}

TEST(Allocation, empty)
{
    t::setupEnv();
    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto vmsToAllocateResult = pgTaskStorage.getVmsToAllocate(5);
    EXPECT_TRUE(vmsToAllocateResult.IsSuccess());
    const auto& vmsToAllocate = vmsToAllocateResult.ValueRefOrThrow();
    EXPECT_EQ(vmsToAllocate.size(), 0u);
}

TEST(Termination, terminate)
{
    t::setupEnv();
    auto pool = pg::createPool();
    const auto vmId = t::insertVm(pool, VmStatus::PendingTermination);

    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto vmsToTerminateResult = pgTaskStorage.getVmsToTerminate(5);
    EXPECT_TRUE(vmsToTerminateResult.IsSuccess());
    const auto& vmsToTerminate = vmsToTerminateResult.ValueRefOrThrow();
    const std::vector<TerminationPendingVmInfo> expectedVmInfos = {
        TerminationPendingVmInfo{
            .id = vmId,
            .cloudVmId = "cloud vm id",
        },
    };
    EXPECT_EQ(vmsToTerminate, expectedVmInfos);

    const auto vmStatusQuery = toString("SELECT status FROM scheduler.vms WHERE id = ", vmId, ";");
    auto readTxn = pool.readOnlyTransaction();
    const auto firstVmStatusResult = pg::execQuery(vmStatusQuery, *readTxn);
    EXPECT_EQ(vmStatusFromString(firstVmStatusResult[0].at("status").as<std::string>()), VmStatus::Terminating);

    const auto saveTerminationResult = pgTaskStorage.saveVmTerminationResult(vmId);
    EXPECT_TRUE(saveTerminationResult.IsSuccess());

    const auto secondVmStatusResult = pg::execQuery(vmStatusQuery, *readTxn);
    EXPECT_EQ(vmStatusFromString(secondVmStatusResult[0].at("status").as<std::string>()), VmStatus::Terminated);
}

TEST(Termination, empty)
{
    t::setupEnv();
    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto vmsToTerminateResult = pgTaskStorage.getVmsToTerminate(5);
    EXPECT_TRUE(vmsToTerminateResult.IsSuccess());
    const auto& vmsToTerminate = vmsToTerminateResult.ValueRefOrThrow();
    EXPECT_EQ(vmsToTerminate.size(), 0u);
}

TEST(Cancellation, cancelQueued)
{
    t::setupEnv();
    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto taskParameters = t::getThreeJobTaskParameters();
    const auto jobsResult = pgTaskStorage.addTask(taskParameters);
    EXPECT_TRUE(jobsResult.IsSuccess());

    const auto createdJobs = CreatedJobs{
        .taskId = 1,
        .jobIds = {1, 2, 3},
    };
    EXPECT_EQ(jobsResult.ValueRefOrThrow(), createdJobs);

    const auto cancelResult = pgTaskStorage.cancelTask(createdJobs.taskId);
    EXPECT_TRUE(cancelResult.IsSuccess());

    const auto jobStates = pgTaskStorage.getJobStates(createdJobs.taskId);
    EXPECT_TRUE(jobStates.IsSuccess());
    EXPECT_TRUE(std::all_of(
        jobStates.ValueRefOrThrow().cbegin(), jobStates.ValueRefOrThrow().cend(),
        [](const JobState& jobState){ return jobState.status == JobStatus::Cancelled; }));
}

TEST(Cancellation, cancelScheduled)
{
    t::setupEnv();
    PgTaskStorage pgTaskStorage(pg::createPool());

    const auto taskParameters = t::getThreeJobTaskParameters();
    const auto jobsResult = pgTaskStorage.addTask(taskParameters);
    EXPECT_TRUE(jobsResult.IsSuccess());

    auto pool = pg::createPool();
    auto writeTxn = pool.writableTransaction();
    const auto updateJobsQuery = toString(
        "UPDATE scheduler.jobs ",
        "SET status = '", toString(JobStatus::Scheduled), "' ",
        "WHERE id IN (", joinSeq(jobsResult.ValueRefOrThrow().jobIds), ");");
    pg::execQuery(updateJobsQuery, *writeTxn);
    writeTxn->commit();

    const auto cancelResult = pgTaskStorage.cancelTask(jobsResult.ValueRefOrThrow().taskId);
    EXPECT_TRUE(cancelResult.IsSuccess());

    const auto jobStates = pgTaskStorage.getJobStates(jobsResult.ValueRefOrThrow().taskId);
    EXPECT_TRUE(jobStates.IsSuccess());
    EXPECT_TRUE(std::all_of(
        jobStates.ValueRefOrThrow().cbegin(), jobStates.ValueRefOrThrow().cend(),
        [](const JobState& jobState){ return jobState.status == JobStatus::Cancelling; }));
}
