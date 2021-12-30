#pragma once

#include <cstddef>

namespace vm_scheduler::allocator {

class Slot {
    size_t cpu;
    size_t memory;
};

} // namespace vm_scheduler::allocator