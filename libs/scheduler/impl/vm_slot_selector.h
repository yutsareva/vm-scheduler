#pragma once

#include <libs/state/include/state.h>

#include <cassert>

namespace vm_scheduler {

class VmSlotSelector {
public:
    VmSlotSelector(std::vector<SlotCapacity> possibleSlots)
        : possibleSlots_(std::move(possibleSlots))
    {
        std::sort(possibleSlots.begin(), possibleSlots.end());
        for (size_t i = 0; i + 1 < possibleSlots.size(); ++i) {
            assert(
                possibleSlots[i].cpu.count() ==
                2 * possibleSlots[i + 1].cpu.count());
            assert(
                possibleSlots[i].ram.count() ==
                2 * possibleSlots[i + 1].ram.count());
        }
    }

    std::pair<JobToVm, DesiredSlotMap> select(std::vector<QueuedJobInfo> jobs)
    {
        std::sort(jobs.begin(), jobs.end());
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
                    (desiredSlots.rbegin() + 1)->first.total;
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
            desiredSlotMap[nextDesiredSlotId] = desiredSlot;
            for (const auto jobId: jobIds) {
                assignments[jobId] = nextDesiredSlotId;
            }
            nextDesiredSlotId = DesiredSlotId{nextDesiredSlotId.value + 1};
        }

        return {std::move(assignments), std::move(desiredSlotMap)};
    }

private:
    SlotCapacity getMinFitSlot(const SlotCapacity& slot) {
        const auto it = std::lower_bound(
            possibleSlots_.begin(),
            possibleSlots_.end(),
            [](const SlotCapacity& lhs, const SlotCapacity& rhs) {
                return lhs.fits(rhs);
            });
        if (it == possibleSlots_.end()) {
            return *possibleSlots_.rbegin();
        }
        return *it;
    }

    std::vector<SlotCapacity> possibleSlots_;
};

} // namespace vm_scheduler
