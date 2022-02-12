#pragma once

#include "libs/common/include/types.h"


namespace vm_scheduler {

struct SlotCapacity {
    CpuCores cpu;
    MegaBytes ram;

    auto operator<=>(const SlotCapacity&) const = default;
};

} // namespace vm_scheduler
