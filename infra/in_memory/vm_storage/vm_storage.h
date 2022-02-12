#pragma once

#include "lib/vm_storage/vm_storage.h"

#include <algorithm>

namespace vm_scheduler::vm_storage {

class InMemoryVmStorage : public VmStorage {
public:
    void save(const AllocatedVm& vmInfo) override { allocated_vms_.emplace_back(vmInfo); }
    std::vector<IdleSlot> getSortedIdleSlots(const size_t limit) override;

private:
    std::vector<AllocatedVm> allocated_vms_;
};

} // namespace vm_scheduler::task_storage
