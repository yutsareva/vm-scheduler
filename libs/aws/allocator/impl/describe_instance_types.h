#pragma once

#include "libs/aws/allocator/include/config.h"

#include "libs/common/include/result.h"

namespace vm_scheduler {

Result<InstanceTypeInfos> describeInstanceTypes(
    Aws::EC2::EC2Client& client, const std::string& instanceClass);

}
