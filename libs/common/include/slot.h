#pragma once

#include "libs/common/include/hash.h"
#include "libs/common/include/types.h"

namespace vm_scheduler {

struct SlotCapacity {
    CpuCores cpu;
    MegaBytes ram;

    auto operator<=>(const SlotCapacity&) const = default;
    bool fits(const SlotCapacity& other) const
    {
        return (cpu <= other.cpu) && (ram <= other.ram);
    }

    SlotCapacity& operator+=(const SlotCapacity& other)
    {
        cpu = CpuCores(cpu.count() + other.cpu.count());
        ram = MegaBytes(ram.count() + other.ram.count());
        return *this;
    }

    SlotCapacity& operator-=(const SlotCapacity& other)
    {
        cpu = CpuCores(std::max(
            static_cast<int>(cpu.count()) - static_cast<int>(other.cpu.count()),
            0));
        ram = MegaBytes(std::max(
            static_cast<int>(ram.count()) - static_cast<int>(other.ram.count()),
            0));
        return *this;
    }

    SlotCapacity& operator*=(const size_t multiplier)
    {
        cpu = CpuCores(cpu.count() * multiplier);
        ram = MegaBytes(ram.count() * multiplier);
        return *this;
    }

    SlotCapacity operator*(const size_t multiplier) const
    {
        return SlotCapacity{
            .cpu = CpuCores(cpu.count() * multiplier),
            .ram = MegaBytes(ram.count() * multiplier),
        };
    }

    SlotCapacity operator-(const SlotCapacity& other) const
    {
        SlotCapacity slot = *this;
        slot -= other;
        return slot;
    }
};

std::ostream& operator<<(std::ostream& out, const SlotCapacity& capacity);

} // namespace vm_scheduler

template<>
struct std::hash<vm_scheduler::SlotCapacity> {
    size_t operator()(const vm_scheduler::SlotCapacity& slot) const
    {
        std::size_t result = 0;
        vm_scheduler::hash_combine(result, slot.cpu.count());
        vm_scheduler::hash_combine(result, slot.ram.count());
        return result;
    }
};
