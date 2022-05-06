#include "libs/aws/allocator/include/cloud_client.h"

#include <libs/common/include/log.h>

#include <gtest/gtest.h>

#include <cstdlib>

using namespace vm_scheduler;

// To run the test AWS_EC2_ACCESS_KEY_ID and AWS_EC2_SECRET_KEY env vars must be specified
// If tests fail check there is no allocated by tests resources left in your AWS account
TEST(AwsCloudClient, simple)
{
    if (!std::getenv("AWS_EC2_ACCESS_KEY_ID") ||
        !std::getenv("AWS_EC2_SECRET_KEY")) {
        INFO() << "Test aws_cloud_client::simple was not launched. "
               << "Please, specify AWS_EC2_ACCESS_KEY_ID and AWS_EC2_SECRET_KEY env vars.";
        return;
    }

    // Allocate the cheapest class of instances in tests (for ARM)
    setenv("VMS_AWS_INSTANCE_CLASS", "a1.", true);

    AwsCloudClient client;

    const auto allocatedVmsAtStart = client.getAllAllocatedVms().ValueOrThrow();
    INFO() << "All allocated VMs at the beginning: " << allocatedVmsAtStart;

    const SlotCapacity slot = {
        .cpu = 1_cores,
        .ram = 1024_MB,
    };

    const auto allocated = client.allocate(time(0), slot);
    EXPECT_TRUE(allocated.IsSuccess()) << what(allocated.ErrorRefOrThrow());

    const auto allocatedVmsAfterAllocation =
        client.getAllAllocatedVms().ValueOrThrow();
    INFO() << "All allocated VMs after allocation: "
           << allocatedVmsAfterAllocation;

    const auto terminated = client.terminate(allocated.ValueRefOrThrow().id);
    EXPECT_TRUE(terminated.IsSuccess()) << what(terminated.ErrorRefOrThrow());

    const auto terminatedAgain = client.terminate(allocated.ValueRefOrThrow().id);
    EXPECT_TRUE(terminatedAgain.IsSuccess())
        << what(terminatedAgain.ErrorRefOrThrow());

    EXPECT_TRUE(
        std::find_if(
            allocatedVmsAfterAllocation.begin(),
            allocatedVmsAfterAllocation.end(),
            [&allocated](const AllocatedVmInfo& a) {
                return a == allocated.ValueRefOrThrow();
            }) != allocatedVmsAfterAllocation.end());
}
