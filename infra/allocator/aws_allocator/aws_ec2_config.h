#pragma once

#include <aws/core/Aws.h>
#include <aws/ec2/model/InstanceType.h>

namespace vm_scheduler::allocator {

struct AwsEc2Config {
    Aws::String amiId;
    std::vector<Aws::EC2::Model::InstanceType> instanceTypes;
};

} // namespace vm_scheduler::allocator