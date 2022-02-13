#pragma once

#include <libs/common/include/slot.h>
#include <libs/common/include/types.h>


namespace vm_scheduler {

using VmId = int64_t;
using CloudVmId = std::string;
using VmType = std::string;

struct AllocatedVmInfo {
    CloudVmId id;
    VmType type;

    bool operator==(const AllocatedVmInfo&) const = default;
};

struct AllocationPendingVmInfo {
    VmId id;
    SlotCapacity capacity;

    bool operator==(const AllocationPendingVmInfo&) const = default;
};

struct TerminationPendingVmInfo {
    VmId id;
    CloudVmId cloudVmId;

    bool operator==(const TerminationPendingVmInfo&) const = default;
};

enum class VmStatus {
    PendingAllocation,
    Allocating,
    Allocated,
    PendingTermination,
    Terminating,
    Terminated,
};

struct ActiveVm { // status: pending_allocation or allocated
    VmId id;
    SlotCapacity totalCapacity;
    SlotCapacity idleCapacity;

    bool operator==(const ActiveVm&) const = default;
};

} // namespace vm_scheduler