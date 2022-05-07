#include <libs/scheduler/impl/vm_slot_selector.h>

#include <cassert>

namespace vm_scheduler {

VmSlotSelector::VmSlotSelector(const std::vector<SlotCapacity>& possibleSlots)
    : possibleSlots_(possibleSlots)
{
    assert(!possibleSlots.empty());
    assert(std::is_sorted(possibleSlots.begin(), possibleSlots.end()));
    for (size_t i = 0; i + 1 < possibleSlots.size(); ++i) {
        assert(
            2 * possibleSlots[i].cpu.count() == possibleSlots[i + 1].cpu.count());
        assert(
            2 * possibleSlots[i].ram.count() == possibleSlots[i + 1].ram.count());
    }
}

std::pair<JobToVm, DesiredSlotMap> VmSlotSelector::select(
    std::vector<QueuedJobInfo> jobs)
{
    std::sort(
        jobs.begin(),
        jobs.end(),
        [](const QueuedJobInfo& lhs, const QueuedJobInfo& rhs) {
            return lhs.requiredCapacity > rhs.requiredCapacity;
        });
    std::vector<std::pair<DesiredSlot, std::vector<JobId>>> desiredSlots;

    for (const auto& job: jobs) {
        std::vector<std::pair<DesiredSlot, std::vector<JobId>>>::iterator
            bestFitDesiredSlot = desiredSlots.end();
        for (auto it = desiredSlots.begin(); it != desiredSlots.end(); ++it) {
            if (job.requiredCapacity.fits(it->first.idle)) {
                if (bestFitDesiredSlot == desiredSlots.end()) {
                    bestFitDesiredSlot = it;
                } else if (bestFitDesiredSlot->first.idle < it->first.idle) {
                    bestFitDesiredSlot = it;
                }
            }
        }

        if (bestFitDesiredSlot != desiredSlots.end()) {
            bestFitDesiredSlot->second.push_back(job.id);
        } else {
            const auto slot = getMinFitSlot(job.requiredCapacity);
            const auto desiredSlot = DesiredSlot{
                .total = slot,
                .idle = slot - job.requiredCapacity,
            };
            desiredSlots.push_back({desiredSlot, {job.id}});
        }

        while (desiredSlots.size() >= 2 &&
               desiredSlots.rbegin()->first.total ==
                   (desiredSlots.rbegin() + 1)->first.total &&
               desiredSlots.rbegin()->first.total.cpu.count() <
                   possibleSlots_.back().cpu.count()) {
            (desiredSlots.rbegin() + 1)->first.idle +=
                (desiredSlots.rbegin())->first.idle;
            (desiredSlots.rbegin() + 1)->first.total *= 2;
            (desiredSlots.rbegin() + 1)
                ->second.insert(
                    (desiredSlots.rbegin() + 1)->second.end(),
                    (desiredSlots.rbegin())->second.begin(),
                    (desiredSlots.rbegin())->second.end());
            desiredSlots.pop_back();
        }
    }

    DesiredSlotMap desiredSlotMap;
    JobToVm assignments;
    DesiredSlotId nextDesiredSlotId = DesiredSlotId{0};

    for (const auto& [desiredSlot, jobIds]: desiredSlots) {
        desiredSlotMap.emplace(nextDesiredSlotId, desiredSlot);
        for (const auto jobId: jobIds) {
            assignments[jobId] = nextDesiredSlotId;
        }
        nextDesiredSlotId = DesiredSlotId{nextDesiredSlotId.value + 1};
    }

    return {std::move(assignments), std::move(desiredSlotMap)};
}

SlotCapacity VmSlotSelector::getMinFitSlot(const SlotCapacity& slot)
{
    const auto it = std::lower_bound(
        possibleSlots_.begin(),
        possibleSlots_.end(),
        slot,
        [](const SlotCapacity& lhs, const SlotCapacity& rhs) {
            return !rhs.fits(lhs);
        });

    if (it == possibleSlots_.end()) {
        return *possibleSlots_.rbegin();
    }
    return *it;
}

} // namespace vm_scheduler
