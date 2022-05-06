#include "libs/scheduler/impl/complex_vm_assigner.h"

namespace vm_scheduler {

ComplexVmAssigner::ComplexVmAssigner(
        const ComplexVmAssignerConfig& config,
        State state,
        std::vector<SlotCapacity> possibleSlots)
    : orderedJobs_(
          createOrderedJobs(config.jobOrdering, std::move(state.queuedJobs)))
    , jobAllocator_(
          createJobAllocator(config.allocationStrategy, std::move(state.vms)))
    , vmSlotSelector_(std::move(possibleSlots))
{ }

StateChange ComplexVmAssigner::assign() noexcept
{
    JobToVm jobToExistingVms;
    //    vmAssignments.reserve(state_.queuedJobs.size());
    std::vector<QueuedJobInfo> unallocatedJobs;
    for (auto it = orderedJobs_->begin(); it != orderedJobs_->end(); ++it) {
        const auto maybeAssignedVmId =
            jobAllocator_->allocate(*it);
        if (maybeAssignedVmId) {
            jobToExistingVms[it->id] = *maybeAssignedVmId;
        } else {
            unallocatedJobs.push_back(*it);
        }
    }

    auto [jobToVm, desiredSlotMap] = vmSlotSelector_.select(std::move(unallocatedJobs));
    jobToVm.merge(jobToExistingVms);

    return {
        .jobToVm = std::move(jobToVm),
        .desiredSlotMap = std::move(desiredSlotMap),
//        .updatedIdleCapacities = ; TBD
//        .vmsToTerminate = ; TBD
    };
}

} // namespace vm_scheduler
