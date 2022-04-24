#include "libs/allocator/include/allocator.h"
#include "libs/allocator/include/cloud_client_mock.h"

#include <libs/common/include/errors.h>
#include <libs/task_storage/include/task_storage_mock.h>

#include <gtest/gtest.h>

using namespace vm_scheduler;
namespace t = vm_scheduler::testing;
using namespace ::testing;

TEST(Allocate, allocate)
{
    t::TaskStorageMock taskStorageMock;
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();

    const auto allocatedVmInfo = AllocatedVmInfo{
        .id = "cloud vm id",
        .type = "cloud vm type",
    };
    EXPECT_CALL(*cloudClientMock, allocate(_, _))
        .WillOnce(Return(Result{allocatedVmInfo}));
    const std::vector<AllocationPendingVmInfo> vmsToAllocate = {
        {.id = 5,
         .capacity = {
             .cpu = 1_cores,
             .ram = 1_MB,
         }}};
    EXPECT_CALL(taskStorageMock, getVmsToAllocate(Eq(1u)))
        .WillOnce(Return(Result{vmsToAllocate}));
    EXPECT_CALL(taskStorageMock, saveVmAllocationResult(Eq(5), Eq(allocatedVmInfo)))
        .WillOnce(Return(Result<void>::Success()));

    Allocator allocator(&taskStorageMock, std::move(cloudClientMock));
    allocator.allocate();
}

TEST(Allocate, allocationFailedOnStart)
{
    t::TaskStorageMock taskStorageMock;
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();

    EXPECT_CALL(taskStorageMock, getVmsToAllocate(Eq(1u)))
        .WillOnce(Return(
            Result<std::vector<AllocationPendingVmInfo>>::Failure<RuntimeException>(
                "Unexpected exception while getting pending allocation vms")));
    EXPECT_CALL(taskStorageMock, saveVmAllocationResult(_, _)).Times(0);
    EXPECT_CALL(taskStorageMock, rollbackUnallocatedVmsState(_, _, _)).Times(0);
    EXPECT_CALL(*cloudClientMock, allocate(_, _)).Times(0);

    Allocator allocator(&taskStorageMock, std::move(cloudClientMock));
    allocator.allocate();
}

TEST(Allocate, allocationFailed)
{
    t::TaskStorageMock taskStorageMock;
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();

    const std::vector<AllocationPendingVmInfo> vmsToAllocate = {
        {.id = 5,
         .capacity = {
             .cpu = 1_cores,
             .ram = 1_MB,
         }}};
    EXPECT_CALL(taskStorageMock, getVmsToAllocate(Eq(1u)))
        .WillOnce(Return(Result{vmsToAllocate}));
    EXPECT_CALL(*cloudClientMock, allocate(_, _))
        .WillOnce(Return(Result<AllocatedVmInfo>::Failure<RuntimeException>(
            "Unexpected exception while requesting a VM")));
    EXPECT_CALL(
        taskStorageMock,
        rollbackUnallocatedVmsState(Eq(std::vector<VmId>{5}), _, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(taskStorageMock, saveVmAllocationResult(_, _)).Times(0);

    Allocator allocator(&taskStorageMock, std::move(cloudClientMock));
    allocator.allocate();
}

TEST(Terminate, terminate)
{
    t::TaskStorageMock taskStorageMock;
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();

    const std::vector<TerminationPendingVmInfo> vmsToTerminate = {{
        .id = 5,
        .cloudVmId = "cloud vm id",
    }};
    EXPECT_CALL(taskStorageMock, getVmsToTerminate(Eq(1u)))
        .WillOnce(Return(Result{vmsToTerminate}));
    EXPECT_CALL(taskStorageMock, saveVmTerminationResult(Eq(5)))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(*cloudClientMock, terminate(Eq(vmsToTerminate[0].cloudVmId)))
        .WillOnce(Return(Result<void>::Success()));

    Allocator allocator(&taskStorageMock, std::move(cloudClientMock));
    allocator.terminate();
}

TEST(Terminate, terminationFailedOnStart)
{
    t::TaskStorageMock taskStorageMock;
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();

    EXPECT_CALL(taskStorageMock, getVmsToTerminate(Eq(1u)))
        .WillOnce(Return(
            Result<std::vector<TerminationPendingVmInfo>>::Failure<RuntimeException>(
                "Unexpected exception while getting pending termination vms")));
    EXPECT_CALL(taskStorageMock, returnUnterminatedVms(_)).Times(0);
    EXPECT_CALL(*cloudClientMock, terminate(_)).Times(0);

    Allocator allocator(&taskStorageMock, std::move(cloudClientMock));
    allocator.terminate();
}

TEST(Terminate, terminationFailed)
{
    t::TaskStorageMock taskStorageMock;
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();

    const std::vector<TerminationPendingVmInfo> vmsToTerminate = {{
        .id = 5,
        .cloudVmId = "cloud vm id",
    }};
    EXPECT_CALL(taskStorageMock, getVmsToTerminate(Eq(1u)))
        .WillOnce(Return(Result{vmsToTerminate}));
    EXPECT_CALL(*cloudClientMock, terminate(Eq(vmsToTerminate[0].cloudVmId)))
        .WillOnce(Return(Result<void>::Failure<RuntimeException>(
            "Unexpected exception while VM termination")));

    EXPECT_CALL(taskStorageMock, returnUnterminatedVms(Eq(std::vector<VmId>{5})))
        .WillOnce(Return(Result<void>::Success()));

    Allocator allocator(&taskStorageMock, std::move(cloudClientMock));
    allocator.terminate();
}
