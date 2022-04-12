#pragma once

#include "libs/common/include/types.h"


namespace vm_scheduler {

struct SlotCapacity {
    CpuCores cpu;
    MegaBytes ram;

    auto operator<=>(const SlotCapacity&) const = default;
};

std::ostream& operator<<(std::ostream& out, const SlotCapacity& capacity);

} // namespace vm_scheduler
