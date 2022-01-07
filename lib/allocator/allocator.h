#pragma once

#include "slot.h"

#include <lib/common/result.h>

#include <string>

namespace vm_scheduler::allocator {

using InstanceType = std::string;
using InstanceId = std::string;

struct InstanceInfo {
    InstanceType type;
    InstanceId id;
    // AllocatorType: aws, azure
    // InstanceState: created, started, stopped, terminated
};

class VmAllocator {
    virtual Result<InstanceInfo> allocate(const Slot& slot) = 0;
    virtual Result<void> deallocate(const InstanceInfo& instanceInfo) = 0;
};

} // namespace vm_scheduler::aws
