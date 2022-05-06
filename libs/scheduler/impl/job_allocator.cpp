#include "libs/scheduler/impl/job_allocator.h"

namespace vm_scheduler {

FirstFit::FirstFit(std::vector<ActiveVm> vms) : vms_(std::move(vms)) { }

std::optional<VmId> FirstFit::allocate(const QueuedJobInfo& job)
{
    for (auto& vm: vms_) {
        if (job.requiredCapacity.fits(vm.idleCapacity)) {
            vm.idleCapacity -= job.requiredCapacity;
            return vm.id;
        }
    }
    return std::nullopt;
}

NextFit::NextFit(std::vector<ActiveVm> vms)
    : vms_(std::move(vms)), iter_(vms.begin())
{ }

std::optional<VmId> NextFit::allocate(const QueuedJobInfo& job)
{
    const auto startIter = iter_;
    while (iter_ != vms_.end()) {
        if (job.requiredCapacity.fits(iter_->idleCapacity)) {
            iter_->idleCapacity -= job.requiredCapacity;
            return iter_->id;
        }
    }
    iter_ = vms_.begin();
    while (iter_ != startIter) {
        if (job.requiredCapacity.fits(iter_->idleCapacity)) {
            iter_->idleCapacity -= job.requiredCapacity;
            return iter_->id;
        }
    }
    return std::nullopt;
}

WorstFit::WorstFit(std::vector<ActiveVm> vms)
{
    std::sort(vms.rbegin(), vms.rend(), [](const ActiveVm& lhs, const ActiveVm& rhs) {
        return rhs.idleCapacity < lhs.idleCapacity;
    });
    vms_ = {vms.begin(), vms.end()}; // TODO: move
}

std::optional<VmId> WorstFit::allocate(const QueuedJobInfo& job)
{
    auto prevIt = vms_.before_begin();
    for (auto it = vms_.begin(); it != vms_.end(); ++it) {
        if (job.requiredCapacity.fits(it->idleCapacity)) {
            it->idleCapacity -= job.requiredCapacity;
            auto activeVm = std::move(*it);
            auto nextIt = vms_.erase_after(prevIt);

            while (nextIt != vms_.end()) {
                if (activeVm.idleCapacity >= nextIt->idleCapacity) {
                    auto insertedIt =
                        vms_.insert_after(prevIt, std::move(activeVm));
                    return insertedIt->id;
                }
                ++prevIt;
                ++nextIt;
            }
        }
        prevIt = it;
    }
    return std::nullopt;
}

BestFit::BestFit(std::vector<ActiveVm> vms)
{
    std::sort(vms.begin(), vms.end(), [](const ActiveVm& lhs, const ActiveVm& rhs) {
            return lhs.idleCapacity < rhs.idleCapacity;
        });
    vms_ = {vms.begin(), vms.end()}; // TODO: move
}

std::optional<VmId> BestFit::allocate(const QueuedJobInfo& job)
{
    for (auto it = vms_.begin(); it != vms_.end(); ++it) {
        if (job.requiredCapacity.fits(it->idleCapacity)) {
            it->idleCapacity -= job.requiredCapacity;
            auto activeVm = std::move(*it);
            auto lastIt = vms_.erase(it);

            for (auto curIt = vms_.begin(); curIt != lastIt; ++curIt) {
                if (curIt->idleCapacity >= activeVm.idleCapacity) {
                    auto insertedIt = vms_.insert(curIt, std::move(activeVm));
                    return insertedIt->id;
                }
            }
            auto insertedIt = vms_.insert(lastIt, std::move(activeVm));
            return insertedIt->id;
        }
    }
    return std::nullopt;
}

} // namespace vm_scheduler
