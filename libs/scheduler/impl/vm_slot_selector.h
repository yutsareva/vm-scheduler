#pragma once

#include <libs/state/include/state.h>

#include <vector>

namespace vm_scheduler {

class VmSlotSelector {
public:
    VmSlotSelector(const std::vector<SlotCapacity>& possibleSlots);
    std::pair<JobToVm, DesiredSlotMap> select(std::vector<QueuedJobInfo> jobs);

private:
    SlotCapacity getMinFitSlot(const SlotCapacity& slot);

private:
    const std::vector<SlotCapacity>& possibleSlots_;
};

} // namespace vm_scheduler
