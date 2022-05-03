#include "libs/scheduler/impl/simple_vm_assigner.h"

namespace vm_scheduler {

ComplexVmAssigner::ComplexVmAssigner(const ComplexVmAssignerConfig& config, State state)
    : orderedJobs_(createOrderedJobs(config.jobOrdering, std::move(state.queuedJobs)))
    , jobAllocator_(createJobAllocator(config.allocationStrategy, std::move(state.vms)))
{ }

StateChange ComplexVmAssigner::assign() noexcept
{
    JobToVm vmAssignments;
//    vmAssignments.reserve(state_.queuedJobs.size());
    std::vector<OrderedJobs::iterator> unallocatedJobs;
    for (it = orderedJobs_.begin(); it != orderedJobs_.end(); ++it) {
        const auto maybeAssignedVmId = jobAllocator_.allocate(job.requiredCapacity);
        if (maybeAssignedVmId) {
            vmAssignments[job.id] = *maybeAssignedVmId;
        } else {
            unallocatedJobs.push_back(it);
        }
    }
//    std::vector<VmId> vmsToTerminate;
//    for (const auto& vm: state_.vms) {
//        if (vm.totalCapacity == vm.idleCapacity) {
//            vmsToTerminate.push_back(vm.id);
//        }
//    }
//
//    return {
//        .vmAssignments = vmAssignments,
//        .vmsToTerminate = vmsToTerminate,
//        .vmCapacityUpdates = {},
//    };
//
//    return {};
}

} // namespace vm_scheduler
