#pragma once

#include <lib/common/slot.h>
#include <lib/common/vm.h>

namespace vm_scheduler::vm_storage {

struct IdleSlot {
    VmId id;
    //    Cloud cloud;
    SlotCapacity capacity;

    bool operator<(const IdleSlot& other) { return capacity < other.capacity; }
};

class VmStorage {
public:
    virtual void save(const AllocatedVm& vmInfo) = 0;
    virtual std::vector<IdleSlot> getSortedIdleSlots(const size_t limit) = 0;
};

} // namespace vm_scheduler::vm_storage
