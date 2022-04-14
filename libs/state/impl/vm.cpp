#include <libs/state/include/vm.h>

#include <iostream>
#include <vector>


namespace vm_scheduler {

const char* toString(VmStatus v)
{
    switch (v) {
        case VmStatus::PendingAllocation : return "pending_allocation";
        case VmStatus::Allocating : return "allocating";
        case VmStatus::Allocated : return "allocated";
        case VmStatus::AgentStarted : return "agent_started";
        case VmStatus::PendingTermination : return "pending_termination";
        case VmStatus::Terminating : return "terminating";
        case VmStatus::Terminated : return "terminated";
    };
}

const std::vector<VmStatus>& getActiveVmStatuses()
{
    static const std::vector<VmStatus> activeVmStatuses =  {
        VmStatus::PendingAllocation,
        VmStatus::Allocating,
        VmStatus::Allocated,
    };
    return activeVmStatuses;
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
