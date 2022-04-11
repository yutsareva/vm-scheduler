#include <libs/state/include/vm.h>

#include <iostream>
#include <vector>


namespace vm_scheduler {

std::vector<VmStatus> getActiveVmStatuses()
{
    return {
        VmStatus::PendingAllocation,
        VmStatus::Allocating,
        VmStatus::Allocated,
    };
}

std::ostream& operator<<(std::ostream& out, const std::vector<ActiveVm>& activeVms)
{
    for (const auto vm : activeVms) {
        out << "[id: " << vm.id << ", "
            << "total capacity: " << vm.totalCapacity << ", "
            << "idle capacity: " << vm.idleCapacity << "], ";
    }
    return out;
}

} // namespace vm_scheduler
