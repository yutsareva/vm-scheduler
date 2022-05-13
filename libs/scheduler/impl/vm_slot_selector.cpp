#include <libs/scheduler/impl/vm_slot_selector.h>

#include <cassert>

namespace vm_scheduler {

VmSlotSelector::VmSlotSelector(const std::vector<SlotCapacity>& possibleSlots)
    : possibleSlots_(possibleSlots)
{
    assert(!possibleSlots.empty());
    assert(std::is_sorted(possibleSlots.begin(), possibleSlots.end()));
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

    DesiredSlotsWithJobs desiredSlots;
    std::unordered_multimap<SlotCapacity, DesiredSlotsWithJobs::iterator>
        totalSlotSizeToDesiredSlots;

    for (const auto& job: jobs) {
        DesiredSlotsWithJobs::iterator bestFitDesiredSlot = desiredSlots.end();
        for (auto it = desiredSlots.begin(); it != desiredSlots.end(); ++it) {
            if ((job.requiredCapacity.fits(it->first.idle)) &&
                (bestFitDesiredSlot == desiredSlots.end() ||
                 (bestFitDesiredSlot->first.idle > it->first.idle))) {
                bestFitDesiredSlot = it;
            }
        }

        if (bestFitDesiredSlot != desiredSlots.end()) {
            bestFitDesiredSlot->second.push_back(job.id);
            bestFitDesiredSlot->first.idle -= job.requiredCapacity;
        } else {
            const auto slot = getMinFitSlot_(job.requiredCapacity);
            const auto desiredSlot = DesiredSlot{
                .total = slot,
                .idle = slot - job.requiredCapacity,
            };
            auto inserted =
                desiredSlots.insert(desiredSlots.end(), {desiredSlot, {job.id}});
            totalSlotSizeToDesiredSlots.insert({slot, inserted});
            merge_(desiredSlots, totalSlotSizeToDesiredSlots, slot);
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

SlotCapacity VmSlotSelector::getMinFitSlot_(const SlotCapacity& slot)
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

void VmSlotSelector::merge_(
    DesiredSlotsWithJobs& desiredSlots,
    std::unordered_multimap<SlotCapacity, DesiredSlotsWithJobs::iterator>&
        totalSlotSizeToDesiredSlots,
    const SlotCapacity& startSlot)
{
    if (totalSlotSizeToDesiredSlots.count(startSlot) == 1) {
        return;
    }
    auto x2Slot =
        std::find(possibleSlots_.begin(), possibleSlots_.end(), startSlot * 2);
    if (x2Slot == possibleSlots_.end()) {
        return;
    }
    auto firstSlot = totalSlotSizeToDesiredSlots.find(startSlot);
    auto newDesiredSlotPair = std::move(*(firstSlot->second));
    desiredSlots.erase(firstSlot->second);
    totalSlotSizeToDesiredSlots.erase(firstSlot);

    auto secondSlot = totalSlotSizeToDesiredSlots.find(startSlot);
    newDesiredSlotPair.first.idle += secondSlot->second->first.idle;
    newDesiredSlotPair.first.total *= 2;
    newDesiredSlotPair.second.insert(
        newDesiredSlotPair.second.end(),
        secondSlot->second->second.begin(),
        secondSlot->second->second.end());
    desiredSlots.erase(secondSlot->second);
    totalSlotSizeToDesiredSlots.erase(secondSlot);

    auto inserted = desiredSlots.insert(desiredSlots.end(), newDesiredSlotPair);
    totalSlotSizeToDesiredSlots.insert({*x2Slot, inserted});
    merge_(desiredSlots, totalSlotSizeToDesiredSlots, *x2Slot);
}

} // namespace vm_scheduler
