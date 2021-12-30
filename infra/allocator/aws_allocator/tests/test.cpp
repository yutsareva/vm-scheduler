#include "infra/allocator/aws_allocator/aws_allocator.h"

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(Test5)
{
    AwsAllocator a;

    BOOST_CHECK_EQUAL(5, 1 + 4);
}

