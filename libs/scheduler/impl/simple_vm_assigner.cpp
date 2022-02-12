#include "libs/scheduler/impl/simple_vm_assigner.h"


namespace vm_scheduler {

StateChange SimpleVmAssigner::assign() noexcept
{
    TaskToVm vmAssignments;
    vmAssignments.reserve(state_.queuedTasks.size());

    for (const auto& task : state_.queuedTasks) {
        vmAssignments[task.id] = DesiredSlot{
            .total = task.requiredCapacity,
            .idle = { 0_cores, 0_MB },
        };
    }
    std::vector<VmId> vmsToTerminate;
    for (const auto& vm : state_.vms) {
        if (vm.totalCapacity == vm.idleCapacity) {
            vmsToTerminate.push_back(vm.id);
        }
    }

    return {
        .vmAssignments = vmAssignments,
        .vmsToTerminate = vmsToTerminate,
        .vmCapacityUpdates = {},
    };
}

} // namespace vm_scheduler
