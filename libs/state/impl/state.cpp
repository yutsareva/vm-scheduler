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
    for (const auto& [jobId, vmInfo] : stateChange.jobToVm) {
        out << "job id: " << jobId << " -> "
            << (std::holds_alternative<VmId>(vmInfo)
                    ? toString("existing VM with id ", std::get<VmId>(vmInfo), ", ")
                    : toString("desired VM id: ", std::get<DesiredSlotId>(vmInfo).value, ", "));
    }
    out << "], desired VMs: [";
    for (const auto& [desiredSlotId, desiredSlot] : stateChange.desiredSlotMap) {
        out << "{ id: " << desiredSlotId.value << ", "
            << "total capacity: " << desiredSlot.total << ", "
            << "idle capacity: " << desiredSlot.idle << " }, ";
    }
    out << "], VM capacities updates: [";
    for (const auto& [vmId, capacity] : stateChange.updatedIdleCapacities) {
        out << "{ id: " << vmId << ", "
            << "new idle capacity: " << capacity << " }, ";
    }
    out << "], VMs to terminate: [" << joinSeq(stateChange.vmsToTerminate);
    out << "]";
    return out;
}

} // vm_scheduler
