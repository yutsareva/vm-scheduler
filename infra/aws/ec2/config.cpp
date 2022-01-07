#include "config.h"

#include <lib/common/env.h>

namespace vm_scheduler::allocator {

AwsEc2Config createEc2Config()
{
    return AwsEc2Config{
        .amiId = getFromEnvOrDefault("AWS_EC2_AMI_ID", "ami-05ff14fd02e80a2ad"),
        .instanceTypes = {}
    };
}

} // namespace vm_scheduler::allocator
