#pragma once

#include "libs/scheduler/impl/complex_vm_assigner_config.h"
#include "libs/scheduler/impl/job_allocator.h"
#include "libs/scheduler/impl/ordered_jobs.h"
#include "libs/scheduler/impl/vm_slot_selector.h"
#include "libs/scheduler/include/vm_assigner.h"

#include <memory>

namespace vm_scheduler {

class ComplexVmAssigner : public VmAssigner {
public:
    ComplexVmAssigner(
        const ComplexVmAssignerConfig& config,
        State state,
        std::vector<SlotCapacity> possibleSlots);
    StateChange assign() noexcept override;

private:
    std::unique_ptr<OrderedJobs> orderedJobs_;
    std::unique_ptr<JobAllocator> jobAllocator_;
    VmSlotSelector vmSlotSelector_;
};

} // namespace vm_scheduler
