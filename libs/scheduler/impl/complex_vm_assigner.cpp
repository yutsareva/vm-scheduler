#include "libs/scheduler/impl/max_min_best_fit_vm_assigner.h"

namespace vm_scheduler {

ComplexVmAssigner::ComplexVmAssigner(
    const ComplexVmAssignerConfig& config,
    State state,
    const std::vector<SlotCapacity>& possibleSlots)
    : orderedJobs_(
          createOrderedJobs(config.jobOrdering, std::move(state.queuedJobs)))
    , jobAllocator_(
          createJobAllocator(config.allocationStrategy, std::move(state.vms)))
    , vmSlotSelector_(possibleSlots)
{ }

StateChange ComplexVmAssigner::assign() noexcept
{
    JobToVm jobToExistingVms;
    std::vector<QueuedJobInfo> unallocatedJobs;
    for (auto it = orderedJobs_->begin(); it != orderedJobs_->end(); ++it) {
        const auto maybeAssignedVmId = jobAllocator_->allocate(*it);
        if (maybeAssignedVmId) {
            jobToExistingVms[it->id] = *maybeAssignedVmId;
        } else {
            unallocatedJobs.push_back(*it);
        }
    }

    auto [jobToVm, desiredSlotMap] =
        vmSlotSelector_.select(std::move(unallocatedJobs));
    jobToVm.merge(jobToExistingVms);

    return {
        .jobToVm = std::move(jobToVm),
        .desiredSlotMap = std::move(desiredSlotMap),
        .updatedIdleCapacities = jobAllocator_->getVmsWithUpdatedCapacities(),
        .vmsToTerminate = jobAllocator_->getIdleVms(),
    };
}

} // namespace vm_scheduler
