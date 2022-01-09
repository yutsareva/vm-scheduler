#pragma once

#include "lib/vm_storage/vm_storage.h"

#include <algorithm>

namespace vm_scheduler::vm_storage {

class InMemoryVmStorage : public VmStorage {
public:
    void save(VmInfo vmInfo) override { allocated_vms_.emplace_back(std::move(vmInfo)); }
    std::vector<IdleSlot> getSortedIdleSlots(const size_t limit) override
    {
        std::vector<IdleSlot> idleSlots;
        std::transform(
            allocated_vms_.begin(),
            allocated_vms_.end(),
            std::back_inserter(idleSlots),
            [](const VmInfo& vmInfo) -> IdleSlot {
                return {.id = vmInfo.id, .cloud = vmInfo.cloud, .cpuCores = vmInfo.idleCpuCores, .memory = vmInfo.idleMemory};
            });
        std::sort(idleSlots.begin(), idleSlots.end(), [](const IdleSlot& first, const IdleSlot& second) -> bool {
            if (first.memory < second.memory) {
                return true;
            }
            if (first.memory == second.memory) {
                return first.cpuCores < second.cpuCores;
            }
            return false;
        });

        idleSlots.resize(std::min(limit, idleSlots.size()));
        return idleSlots;
    }

private:
    std::vector<VmInfo> allocated_vms_;
};

} // namespace vm_scheduler::vm_storage
