#pragma once

#include <lib/common/result.h>
#include <lib/common/slot.h>

#include <string>

namespace vm_scheduler::allocator {

using VmType = std::string;
using VmId = std::string;

struct VmInfo {
    VmType type;
    VmId id;
    // AllocatorType: aws, azure
    // VmState: created, started, stopped, terminated
};

class VmAllocator {
    virtual Result<VmInfo> allocate(const SlotCapacity& slot) = 0;
    virtual Result<void> deallocate(const VmInfo& instanceInfo) = 0;
};

} // namespace vm_scheduler::aws
