#pragma once

#include "libs/aws/allocator/include/config.h"
#include <libs/common/include/result.h>
#include <libs/state/include/vm.h>

namespace vm_scheduler {

Result<AllocatedVmInfos> describeInstances(
    Aws::EC2::EC2Client& client, const Tags& tags);

} // namespace vm_scheduler
