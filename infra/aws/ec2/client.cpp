#include "client.h"

#include <lib/common/env.h>

#include <aws/core/auth/AWSCredentials.h>

namespace vm_scheduler::allocator {

namespace {

Aws::Auth::AWSCredentials getCredentials()
{
    const auto accessKeyId = getFromEnvOrThrow("AWS_EC2_ACCESS_KEY_ID");
    const auto secretKey = getFromEnvOrThrow("AWS_EC2_SECRET_KEY");
    const auto sessionToken = "";

    return {accessKeyId, secretKey, sessionToken};
}

} // anonymous namespace

Aws::EC2::EC2Client createEc2Client()
{
    const auto credentials = getCredentials();
    return {credentials};
}

} // namespace vm_scheduler::allocator