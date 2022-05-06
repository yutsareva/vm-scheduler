#pragma once

#include "libs/common/include/types.h"


namespace vm_scheduler {

struct SlotCapacity {
    CpuCores cpu;
    MegaBytes ram;

    auto operator<=>(const SlotCapacity&) const = default;
    bool fits(const SlotCapacity& other) const {
        return cpu < other.cpu && ram < other.ram;
    }

    SlotCapacity& operator+=(const SlotCapacity& other) {
        cpu = CpuCores(cpu.count() + other.cpu.count());
        ram = MegaBytes(ram.count() + other.ram.count());
        return *this;
    }

    SlotCapacity& operator-=(const SlotCapacity& other) {
        cpu = CpuCores(cpu.count() - other.cpu.count());
        ram = MegaBytes(ram.count() - other.ram.count());
        return *this;
    }

    SlotCapacity& operator*=(const size_t multiplier) {
        cpu = CpuCores(cpu.count() * multiplier);
        ram = MegaBytes(ram.count() * multiplier);
        return *this;
    }

    SlotCapacity operator-(const SlotCapacity& other) const {
        return SlotCapacity{
            .cpu = CpuCores(cpu.count() - other.cpu.count()),
            .ram = MegaBytes(ram.count() - other.ram.count()),
        };
    }
};

std::ostream& operator<<(std::ostream& out, const SlotCapacity& capacity);

} // namespace vm_scheduler
