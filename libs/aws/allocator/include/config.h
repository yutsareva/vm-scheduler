#pragma once

#include "libs/common/include/types.h"

#include <libs/common/include/slot.h>

#include <aws/core/Aws.h>
#include <aws/ec2/EC2Client.h>
#include <aws/ec2/model/InstanceType.h>

namespace vm_scheduler {

struct AwsClientConfig {
    std::string accessKeyId;
    std::string secretKey;
    std::string sessionToken = "";
};

AwsClientConfig createAwsClientConfig();

struct InstanceTypeInfo {
    Aws::EC2::Model::InstanceType type;

    SlotCapacity capacity;

    auto operator<(const InstanceTypeInfo& other) const;
};

using InstanceTypeInfos = std::vector<InstanceTypeInfo>;

std::ostream& operator<<(std::ostream& out, const InstanceTypeInfos& vmTypeInfos);

struct AwsInstancesConfig {
    Aws::String amiId;
    InstanceTypeInfos vmTypes;
};

AwsInstancesConfig createAwsInstancesConfig(Aws::EC2::EC2Client& client);

} // namespace vm_scheduler
