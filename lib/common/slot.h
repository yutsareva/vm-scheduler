#pragma once

#include "types.h"

namespace vm_scheduler {

struct SlotCapacity {
    CpuCores cpu;
    GigaBytes memory;
    auto operator<=>(const SlotCapacity&) const = default;
};

} // namespace vm_scheduler
