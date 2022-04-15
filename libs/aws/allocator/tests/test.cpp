#include "libs/aws/allocator/include/cloud_client.h"

#include <libs/common/include/log.h>

#define BOOST_TEST_MODULE aws_allocator
#include <boost/test/unit_test.hpp>

#include <cstdlib>

using namespace vm_scheduler;


// To run the test AWS_EC2_ACCESS_KEY_ID and AWS_EC2_SECRET_KEY env vars must be specified
// If tests fail check there is no allocated by tests resources left in your AWS account
BOOST_AUTO_TEST_CASE(simple) {
    if (!std::getenv("AWS_EC2_ACCESS_KEY_ID") || !std::getenv("AWS_EC2_SECRET_KEY")) {
        INFO() << "Test aws_cloud_client::simple was not launched. "
               << "Please, specify AWS_EC2_ACCESS_KEY_ID and AWS_EC2_SECRET_KEY env vars.";
        return;
    }

    AwsCloudClient client;
    const SlotCapacity slot = {
        .cpu = 1_cores,
        .ram = 1024_MB,
    };

    const auto allocated = client.allocate(slot);
    BOOST_ASSERT(allocated.IsSuccess());

    const TerminationPendingVmInfo vmInfo = {
        .id = 42,
        .cloudVmId = allocated.ValueRefOrThrow().id,
    };

    const auto terminated = client.terminate(vmInfo);
    BOOST_ASSERT(terminated.IsSuccess());
}
