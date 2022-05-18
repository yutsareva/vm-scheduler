#include <libs/state/include/vm.h>

#include <iostream>
#include <vector>
#include <unordered_map>


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

VmStatus vmStatusFromString(const std::string& s)
{
    const static std::unordered_map<std::string, VmStatus> map = {
        {"pending_allocation", VmStatus::PendingAllocation},
        {"allocating", VmStatus::Allocating},
        {"allocated", VmStatus::Allocated},
        {"agent_started", VmStatus::AgentStarted},
        {"pending_termination", VmStatus::PendingTermination},
        {"terminating", VmStatus::Terminating},
        {"terminated", VmStatus::Terminated},
    };
    return map.at(s);
}

template<>
VmStatus fromString(const std::string& s)
{
    return vmStatusFromString(s);
}

const std::unordered_set<VmStatus>& getActiveVmStatuses()
{
    static const std::unordered_set<VmStatus> finalVmStatuses = {
        VmStatus::PendingAllocation,
        VmStatus::Allocating,
        VmStatus::Allocated,
        VmStatus::AgentStarted,
    };
    return finalVmStatuses;
}

std::ostream& operator<<(
    std::ostream& out, const std::vector<ActiveVm>& activeVms)
{
    out << "[";
    for (const auto vm : activeVms) {
        out << "{id: " << vm.id << ", "
            << "total capacity: " << vm.totalCapacity << ", "
            << "idle capacity: " << vm.idleCapacity << "}, ";
    }
    out << "]";
    return out;
}

std::ostream& operator<<(std::ostream& out, const AllocatedVmInfos& vms)
{
    out << "[";
    for (const auto& vm : vms) {
        out << "{id: " << vm.id << ", "
            << "type: " << vm.type << "}, ";
    }
    out << "]";
    return out;
}

} // namespace vm_scheduler
