#pragma once

#include <libs/state/include/state.h>

#include <list>
#include <vector>

namespace vm_scheduler {

using DesiredSlotsWithJobs =
    std::list<std::pair<DesiredSlot, std::vector<JobId>>>;

class VmSlotSelector {
public:
    explicit VmSlotSelector(const std::vector<SlotCapacity>& possibleSlots);
    std::pair<JobToVm, DesiredSlotMap> select(std::vector<QueuedJobInfo> jobs);

private:
    SlotCapacity getMinFitSlot_(const SlotCapacity& slot);
    void merge_(
        DesiredSlotsWithJobs& desiredSlots,
        std::unordered_multimap<SlotCapacity, DesiredSlotsWithJobs::iterator>&
            totalSlotSizeToDesiredSlots,
        const SlotCapacity& startSlot);

private:
    const std::vector<SlotCapacity>& possibleSlots_;
};

} // namespace vm_scheduler
