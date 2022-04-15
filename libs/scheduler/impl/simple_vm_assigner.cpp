#include "libs/scheduler/impl/simple_vm_assigner.h"

namespace vm_scheduler {

StateChange SimpleVmAssigner::assign() noexcept
{
    JobToVm vmAssignments;
    vmAssignments.reserve(state_.queuedJobs.size());

    for (const auto& job: state_.queuedJobs) {
        vmAssignments[job.id] = DesiredSlot{
            .total = job.requiredCapacity,
            .idle = {0_cores, 0_MB},
        };
    }
    std::vector<VmId> vmsToTerminate;
    for (const auto& vm: state_.vms) {
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
