#pragma once

#include <lib/common/slot.h>
#include <lib/common/vm.h>

namespace vm_scheduler::vm_storage {

class VmStorage {
public:
    virtual void save(const AllocatedVm& vmInfo) = 0;
    virtual std::vector<IdleSlot> getSortedIdleSlots(const size_t limit) = 0;
};

} // namespace vm_scheduler::vm_storage
