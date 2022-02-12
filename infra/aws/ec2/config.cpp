#include "config.h"

#include <lib/common/env.h>

namespace vm_scheduler::allocator {

AwsEc2Config createEc2Config()
{
    Aws::EC2::Model::Region	defaultRegion;
    const auto defaultRegionStr = getFromEnvOrDefault("AWS_EC2_REGION", "us-east-2");
    defaultRegion.SetRegionName("us-east-2");
    return AwsEc2Config{
        .amiId = getFromEnvOrDefault("AWS_EC2_AMI_ID", "ami-05ff14fd02e80a2ad"),
        .region = defaultRegion
    };
}

} // namespace vm_scheduler::allocator
