#include "libs/aws/allocator/include/config.h"
#include <libs/common/include/env.h>

namespace vm_scheduler {

namespace {

const std::string DEFAULT_AWS_EC2_AMI_ID = "ami-0d4df4d5d1febee7c";

} // anonymous namespace

AwsEc2Config createEc2Config()
{
    return AwsEc2Config{
        .amiId = getFromEnvOrDefault("AWS_EC2_AMI_ID", DEFAULT_AWS_EC2_AMI_ID),
        .vmTypes = {}, // TODO: implement
        .accessKeyId = getFromEnvOrThrow("AWS_EC2_ACCESS_KEY_ID"),
        .secretKey = getFromEnvOrThrow("AWS_EC2_SECRET_KEY"),
    };
}

} // namespace vm_scheduler
