#include "config.h"

#include <lib/common/env.h>


namespace vm_scheduler::allocator {

AwsEc2Config createEc2Config()
{
    return AwsEc2Config{
        .amiId = getFromEnvOrDefault("AWS_EC2_AMI_ID", "ami-0d4df4d5d1febee7c"),
        .instanceTypes = {}
    };
}

} // namespace vm_scheduler::aws
