#include "libs/scheduler/include/scheduler.h"

#include <libs/common/include/errors.h>
#include <libs/task_storage/include/task_storage_mock.h>

#include <gtest/gtest.h>

using namespace vm_scheduler;
namespace t = vm_scheduler::testing;
using namespace ::testing;

namespace {

std::vector<SlotCapacity> getPossibleSlots()
{
    return {
        {
            .cpu = CpuCores(1),
            .ram = MegaBytes(512),
        },
        {
            .cpu = CpuCores(2),
            .ram = MegaBytes(1024),
        },
        {
            .cpu = CpuCores(4),
            .ram = MegaBytes(2048),
        },
    };
}

} // anonymous namespace

TEST(Scheduler, schedule)
{
    t::TaskStorageMock taskStorageMock;
    Scheduler scheduler("backendId", &taskStorageMock, getPossibleSlots());

    EXPECT_CALL(taskStorageMock, addTask).Times(0);
    EXPECT_CALL(taskStorageMock, startScheduling(_, _))
        .WillOnce(Return(Result{PlanId{456}}));
    EXPECT_CALL(taskStorageMock, getCurrentState())
        .WillOnce(Return(Result{State{}}));
    EXPECT_CALL(taskStorageMock, commitPlanChange(_, _))
        .WillOnce(Return(Result<void>::Success()));

    scheduler.schedule();
}

TEST(Scheduler, schedulingCancelled)
{
    t::TaskStorageMock taskStorageMock;
    Scheduler scheduler("backendId", &taskStorageMock, getPossibleSlots());

    EXPECT_CALL(taskStorageMock, startScheduling(_, _))
        .WillOnce(Return(Result<PlanId>::Failure<RuntimeException>(
            "The plan was recently updated, new scheduling is not needed yet")));
    EXPECT_CALL(taskStorageMock, getCurrentState).Times(0);
    EXPECT_CALL(taskStorageMock, commitPlanChange).Times(0);

    scheduler.schedule();
}
