#pragma once

#include <libs/common/include/slot.h>
#include <libs/common/include/types.h>

#include <chrono>


namespace vm_scheduler {

using VmId = int64_t;
using VmType = std::string;

enum class VmStatus {
    PendingAllocation,
    Allocating,
    Allocated,
    PendingTermination,
    Terminated,
};

struct ActiveVm { // status: pending_allocation or allocated
    VmId id;
    SlotCapacity totalCapacity;
    SlotCapacity idleCapacity;
    std::chrono::time_point<std::chrono::steady_clock> lastFinishedTaskAt;

    bool operator==(const ActiveVm&) const = default;
};

} // namespace vm_scheduler
