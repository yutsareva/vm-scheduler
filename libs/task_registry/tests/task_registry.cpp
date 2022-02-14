#include "libs/task_registry/include/task_registry.h"

#include <libs/task_storage/include/task_storage_mock.h>
#include <libs/allocator/include/cloud_client_mock.h>

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

    EXPECT_CALL(*taskStorageMock, startScheduling(_, _)).WillOnce(::testing::Return(Result{PlanId{456}}));
    EXPECT_CALL(*taskStorageMock, getCurrentState()).WillOnce(::testing::Return(Result{State{}}));
    EXPECT_CALL(*taskStorageMock, commitPlanChange(_, _)).WillOnce(::testing::Return(Result<void>::Success()));
    EXPECT_CALL(*taskStorageMock, getVmsToAllocate(_)).WillOnce(
        Return(Result{std::vector<AllocationPendingVmInfo>{}}));
    EXPECT_CALL(*taskStorageMock, getVmsToTerminate(_)).WillOnce(
        Return(Result{std::vector<TerminationPendingVmInfo>{}}));

    TaskRegistry taskRegistry(config, std::move(taskStorageMock), std::move(cloudClientMock));
    std::this_thread::sleep_for(1s);
}
