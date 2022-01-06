#pragma once

#include <aws/core/Aws.h>
#include <aws/ec2/model/InstanceType.h>

namespace vm_scheduler::allocator {

struct AwsEc2Config {
    Aws::String amiId;
    std::vector<Aws::EC2::Model::InstanceType> instanceTypes;
};

AwsEc2Config createEc2Config();

} // namespace vm_scheduler::aws

