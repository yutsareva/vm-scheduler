#pragma once

#include "slot.h"

namespace vm_scheduler::allocator {

class VmAllocator {
    virtual void allocate(const Slot& slot) = 0;
    virtual void deallocate(const Slot& slot) = 0;
};

} // namespace vm_scheduler::allocator
