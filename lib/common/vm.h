#pragma once

#include <chrono>
#include <string>
#include <variant>
#include <vector>

namespace vm_scheduler {

using VmId = std::string;

enum class Cloud { Aws, Azure };

struct AllocatedVm {
    VmId id;
    Cloud cloud;
    std::chrono::time_point<std::chrono::steady_clock> allocatedAt;
    SlotCapacity totalCapacity;
    SlotCapacity idleCapacity;
    // status
    // type
    // CloudSpecificInfo
};

struct DesiredSlot {
    Cloud cloud;
    SlotCapacity total;
};

using VmInfo = std::variant<VmId, DesiredSlot>;

struct IdleSlot {
    VmId id;
    //    Cloud cloud;
    SlotCapacity capacity;

    bool operator<(const IdleSlot& other) { return capacity < other.capacity; }
};
} // namespace vm_scheduler
