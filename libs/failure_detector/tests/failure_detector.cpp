#include "libs/failure_detector/include/failure_detector.h"

#include <libs/allocator/include/allocator.h>
#include <libs/allocator/include/cloud_client_mock.h>
#include <libs/common/include/errors.h>
#include <libs/task_storage/include/task_storage_mock.h>

#include <gtest/gtest.h>

using namespace vm_scheduler;
namespace t = vm_scheduler::testing;
using namespace ::testing;

TEST(FailureDetector, simple)
{
    t::TaskStorageMock taskStorageMock;
    auto cloudClientMock = std::make_unique<t::CloudClientMock>();

    EXPECT_CALL(*cloudClientMock, getAllAllocatedVms())
        .WillOnce(Return(Result{AllocatedVmInfos{}}));

    Allocator allocator(&taskStorageMock, std::move(cloudClientMock));

    FailureDetector failureDetector(&taskStorageMock, &allocator);

    EXPECT_CALL(taskStorageMock, restartStaleAllocatingVms(_, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(taskStorageMock, terminateStaleAllocatingVms(_, _, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(taskStorageMock, terminateVmsWithInactiveAgents(_, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(taskStorageMock, terminateVmsWithoutAgents(_, _))
        .WillOnce(Return(Result<void>::Success()));
    EXPECT_CALL(taskStorageMock, getAllocatedVms())
        .WillOnce(Return(Result{AllocatedVmInfos{}}));
    EXPECT_CALL(taskStorageMock, cancelTimedOutJobs())
        .WillOnce(Return(Result<void>::Success()));

    failureDetector.monitor();
}
