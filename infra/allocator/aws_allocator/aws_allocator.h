#pragma once

#include "lib/allocator/allocator.h"

namespace vm_scheduler::allocator {

class AwsAllocator : public VmAllocator {
    void allocate(const Slot& slot) override;
    void deallocate(const Slot& slot) override;
};

} // namespace vm_scheduler::allocator
