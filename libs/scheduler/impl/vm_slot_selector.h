#pragma once

#include <libs/state/include/state.h>

namespace vm_scheduler {

class VmSlotSelector {
public:
    VmSlotSelector(std::vector<SlotCapacity> possibleSlots)
        : possibleSlots_(std::move(possibleSlots))
    {
        std::sort(possibleSlots.begin(), possibleSlots.end());
        for (size_t i = 0; i + 1 < possibleSlots.size(); ++i) {
            assert(
                possibleSlots[i].cpu.value == 2 * possibleSlots[i + 1].cpu.value);
            assert(
                possibleSlots[i].ram.value == 2 * possibleSlots[i + 1].ram.value);
        }
    }

    std::pair<JobToVm, DesiredSlotMap> select(std::vector<QueuedJobInfo> jobs)
    {
        std::sort(jobs.begin(), jobs.end());
        std::vector<std::pair<DesiredSlot, std::vector<JobId>>> desiredSlots;

        for (const auto& job: jobs) {
            std::vector < std::pair<DesiredSlot, std::vector<JobId>>::iterator
                              bestFitDesiredSlot = desiredSlots.end();
            for (auto it = desiredSlots.begin(); it != desiredSlots.end(); ++it) {
                if (job.requiredCapacity.fits(it->first.idleCapacity)) {
                    if (bestFitDesiredSlot == desiredSlots.end()) {
                        bestFitDesiredSlot = it;
                    } else if (
                        bestFitDesiredSlot->first.idleCapacity <
                        it->first.idleCapacity) {
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
                desiredSlots.push_back({desiredSlot, job.id});
            }

            while (desiredSlots.size() >= 2 &&
                   desiredSlots.rbegin()->first.totalCapcity ==
                       (desiredSlots.rbegin() + 1)->first.totalCapcity &&
                   desiredSlots.rbegin()->first.totalCapcity.cpu.value <
                       possibleSlots_.back().cpu.value) {
                (desiredSlots.rbegin() + 1)->first.cpu.value *= 2;
                (desiredSlots.rbegin() + 1)->first.ram.value *= 2;
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
        DesiredSlotId nextDesiredSlotId = {0};

        for (const auto& [desiredSlot, jobIds] : desiredSlots) {
            desiredSlotMap[nextDesiredSlotId] = desiredSlot;
            for (const jobId : jobIds) {
                assignments[jobId] = nextDesiredSlotId;
            }
            nextDesiredSlotId = {nextDesiredSlotId.value + 1};
        }

        return {std::move(assignments), std::move(desiredSlotMap)};
    }

private:
    SlotCapacity getMinFitSlot(const SlotCapacity& slot)
    {
        const auto it = std::lower_bound(
            possibleSlots_.begin(), possibleSlots_.end(), SlotCapacity::fits);
        if (it == possibleSlots_.end()) {
            return *possibleSlots_.rbegin();
        }
        return *it;
    }

    std::vector<SlotCapacity> possibleSlots_;
};

} // namespace vm_scheduler
