#pragma once

#include <aws/core/Aws.h>
#include <aws/ec2/model/InstanceType.h>

namespace vm_scheduler {

struct AwsEc2Config {
    Aws::String amiId;
    std::vector<Aws::EC2::Model::InstanceType> vmTypes;
    std::string accessKeyId;
    std::string secretKey;
    std::string sessionToken = "";
};

AwsEc2Config createEc2Config();

} // namespace vm_scheduler
