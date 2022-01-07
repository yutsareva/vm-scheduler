#include "infra/aws/ec2/allocator.h"

#include <lib/common/types.h>
#define BOOST_TEST_MODULE aws_allocator
#include <boost/test/unit_test.hpp>

#include <cstdlib>

using namespace vm_scheduler;
using namespace vm_scheduler::allocator;

BOOST_AUTO_TEST_CASE(TestAllocateDeallocateAwsInstance)
{
    setenv("AWS_EC2_ACCESS_KEY_ID", "put access key id here", 1);
    setenv("AWS_EC2_SECRET_KEY", "put secret key here", 1);

    Aws::SDKOptions options;
    Aws::InitAPI(options);

    AwsAllocator allocator;
    Slot slot = {
        .cpu = 1_cores,
        .memory = 1_GB,
    };

    const auto allocated = allocator.allocate(slot);
    BOOST_TEST(allocated.IsSuccess());
    const auto deallocated = allocator.deallocate(allocated.ValueRefOrThrow());
    BOOST_CHECK(deallocated.IsSuccess());

    Aws::ShutdownAPI(options);
}
