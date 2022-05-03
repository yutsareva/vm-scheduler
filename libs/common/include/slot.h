#pragma once

#include "libs/common/include/types.h"


namespace vm_scheduler {

struct SlotCapacity {
    CpuCores cpu;
    MegaBytes ram;

    auto operator<=>(const SlotCapacity&) const = default;
    bool fits(const SlotCapacity& other) {
        return cpu < other.cpu && ram < other.ram;
    }
};

std::ostream& operator<<(std::ostream& out, const SlotCapacity& capacity);

} // namespace vm_scheduler
