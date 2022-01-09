#include "vm_storage.h"

namespace vm_scheduler::vm_storage {

std::vector<IdleSlot> InMemoryVmStorage::getSortedIdleSlots(const size_t limit) override
{
    std::vector<IdleSlot> idleSlots;
    std::transform(
        allocated_vms_.begin(), allocated_vms_.end(), std::back_inserter(idleSlots), [](const VmInfo& vmInfo) -> IdleSlot {
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

} // namespace vm_scheduler::vm_storage
