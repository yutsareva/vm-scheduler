#include "libs/scheduler/impl/simple_vm_assigner.h"

namespace vm_scheduler {

StateChange SimpleVmAssigner::assign() noexcept
{
    JobToVm vmAssignments;
    vmAssignments.reserve(state_.queuedJobs.size());

    DesiredSlotMap desiredSlotMap;
    DesiredSlotId nextDesiredSlotId = DesiredSlotId{0};
    for (const auto& job: state_.queuedJobs) {
        vmAssignments[job.id] = nextDesiredSlotId;
        desiredSlotMap.emplace(nextDesiredSlotId,DesiredSlot{
            .total = job.requiredCapacity,
            .idle = {0_cores, 0_MB},
        });
        nextDesiredSlotId = DesiredSlotId{nextDesiredSlotId.value + 1};
    }
    std::vector<VmId> vmsToTerminate;
    for (const auto& vm: state_.vms) {
        if (vm.totalCapacity == vm.idleCapacity) {
            vmsToTerminate.push_back(vm.id);
        }
    }

    return StateChange{
        .jobToVm = std::move(vmAssignments),
        .desiredSlotMap = std::move(desiredSlotMap),
        .updatedIdleCapacities = {},
        .vmsToTerminate = vmsToTerminate,
    };
}

} // namespace vm_scheduler
