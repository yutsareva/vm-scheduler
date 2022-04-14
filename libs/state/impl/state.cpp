#include "libs/state/include/state.h"
#include "libs/common/include/stringify.h"

#include <iostream>


namespace vm_scheduler {

std::ostream& operator<<(std::ostream& out, const DesiredSlot& desiredSlot)
{
    out << "total capacity: " << desiredSlot.total << ", "
        << "idle capacity: " << desiredSlot.idle;
    return out;
}

std::ostream& operator<<(std::ostream& out, const State& state)
{
    out << "queued job infos: " << state.queuedJobs
        << "active vms: " << state.vms;
    return out;
}

std::ostream& operator<<(std::ostream& out, const StateChange& stateChange)
{
    out << "job assignments: [";
    for (const auto& [jobId, vmInfo] : stateChange.vmAssignments) {
        out << "job id: " << jobId << " -> "
            << (std::holds_alternative<VmId>(vmInfo)
                    ? toString("existing VM with id ", std::get<VmId>(vmInfo), ", ")
                    : toString("desired VM {", std::get<DesiredSlot>(vmInfo), "}, "));
    }
//    out << "], VMs to terminate: [" << joinSeq(stateChange.vmsToTerminate);
    out << "], VM capacities updates: [";
    for (const auto& vmCapacityUpdate : stateChange.vmCapacityUpdates) {
        out << vmCapacityUpdate << ", ";
    }
    out << "]";
    return out;
}

std::ostream& operator<<(std::ostream& out, const VmCapacityUpdate& vmCapacityUpdate)
{
    out << "id: " << vmCapacityUpdate.id << ", "
        << "idle capacity: " << vmCapacityUpdate.idleCapacity;
    return out;
}

} // vm_scheduler
