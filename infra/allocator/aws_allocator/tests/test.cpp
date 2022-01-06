#include "infra/allocator/aws_allocator/aws_allocator.h"

#include <lib/common/types.h>
//#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE aws_allocator
#include <boost/test/unit_test.hpp>

using namespace vm_scheduler;
using namespace vm_scheduler::allocator;

BOOST_AUTO_TEST_CASE(TestAllocateDeallocateAwsInstance)
{
    AwsAllocator allocator;
    Slot slot = {
        .cpu = 1_cores,
        .memory = 1_GB,
    };
    const auto allocated = allocator.allocate(slot);
    BOOST_CHECK(allocated.IsSuccess());
    const auto deallocated = allocator.deallocate(allocated.ValueRefOrThrow());
    BOOST_CHECK(deallocated.IsSuccess());

}

