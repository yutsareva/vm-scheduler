#include "libs/task_registry/include/task_registry.h"

#include <libs/allocator/include/cloud_client_mock.h>
#include <libs/task_storage/include/task_storage_mock.h>

#include <gtest/gtest.h>

#include <memory>
#include <thread>

using namespace vm_scheduler;
namespace t = vm_scheduler::testing;
using namespace ::testing;
using namespace std::chrono_literals;

TEST(task_registry, simple)
{
    const auto config = createConfig();

    auto taskStorageMock = std::make_unique<t::TaskStorageMock>();
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();

    EXPECT_CALL(*taskStorageMock, startScheduling(_, _))
        .WillOnce(::testing::Return(Result{PlanId{456}}));
    EXPECT_CALL(*taskStorageMock, getCurrentState())
        .WillOnce(::testing::Return(Result{State{}}));
    EXPECT_CALL(*taskStorageMock, commitPlanChange(_, _))
        .WillOnce(::testing::Return(Result<void>::Success()));
    EXPECT_CALL(*taskStorageMock, getVmsToAllocate(_))
        .WillOnce(Return(Result{std::vector<AllocationPendingVmInfo>{}}));
    EXPECT_CALL(*taskStorageMock, getVmsToTerminate(_))
        .WillOnce(Return(Result{std::vector<TerminationPendingVmInfo>{}}));
    EXPECT_CALL(*taskStorageMock, restartStaleAllocatingVms(_, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(*taskStorageMock, terminateStaleAllocatingVms(_, _, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(*taskStorageMock, terminateVmsWithInactiveAgents(_, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(*taskStorageMock, terminateVmsWithoutAgents(_, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(*taskStorageMock, cancelTimedOutJobs())
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(*taskStorageMock, getAllocatedVms())
        .WillOnce(Return(Result{AllocatedVmInfos{}}));
    EXPECT_CALL(*cloudClientMock, getAllAllocatedVms())
        .WillOnce(Return(Result{AllocatedVmInfos{}}));
    EXPECT_CALL(*cloudClientMock, getPossibleSlots())
        .WillOnce(Return(std::vector<SlotCapacity>{
            SlotCapacity{
                .cpu = 1_cores,
                .ram = 1024_MB,
            },
            SlotCapacity{
                .cpu = 2_cores,
                .ram = 2048_MB,
            }
        }));

    TaskRegistry taskRegistry(
        config, std::move(taskStorageMock), std::move(cloudClientMock));
    std::this_thread::sleep_for(1s);
}
