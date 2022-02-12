#include "vm_storage.h"

namespace vm_scheduler::vm_storage {

std::vector<IdleSlot> InMemoryVmStorage::getSortedIdleSlots(const size_t limit)
{
    std::vector<IdleSlot> idleSlots;
    std::transform(
        allocated_vms_.begin(),
        allocated_vms_.end(),
        std::back_inserter(idleSlots),
        [](const AllocatedVm& vmInfo) -> IdleSlot {
            return {.id = vmInfo.id, .capacity = { .cpu = vmInfo.idleCapacity.cpu, .memory = vmInfo.idleCapacity.memory} };
        });
    std::sort(idleSlots.begin(), idleSlots.end());

    const size_t eraseCount = static_cast<size_t>(std::min(0, static_cast<int>(idleSlots.size()) - static_cast<int>(limit)));
    idleSlots.erase(idleSlots.end() - eraseCount, idleSlots.end());

    return idleSlots;
}

} // namespace vm_scheduler::vm_storage
