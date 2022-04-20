#pragma once

#include "libs/aws/allocator/include/config.h"

#include <libs/common/include/types.h>

#include <aws/ec2/model/InstanceType.h>

namespace vm_scheduler {

// public only for testing purposes
Aws::EC2::Model::InstanceType getInstanceTypeForSlot(
    const InstanceTypeInfos& vmTypeInfos, const SlotCapacity& slot);

} // namespace vm_scheduler
