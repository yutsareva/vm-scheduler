#pragma once

#include <lib/common/types.h>

namespace vm_scheduler::allocator {

struct Slot {
    CpuCores cpu;
    GigaBytes memory;
};

} // namespace vm_scheduler::allocator