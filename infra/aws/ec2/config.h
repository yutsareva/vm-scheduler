#pragma once

#include <aws/core/Aws.h>
#include <aws/ec2/model/InstanceType.h>
#include <aws/ec2/model/Region.h>

namespace vm_scheduler::allocator {

struct AwsEc2Config {
    Aws::String amiId;
    Aws::EC2::Model::Region	region;
//        std::vector<Aws::EC2::Model::InstanceType> instanceTypes;
};

AwsEc2Config createEc2Config();

} // namespace vm_scheduler::allocator
