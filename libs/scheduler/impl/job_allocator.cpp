#include "libs/scheduler/impl/job_allocator.h"

namespace vm_scheduler {

FirstFit::FirstFit(std::vector<ActiveVm> vms) override : vms_(std::move(vms)) { }

std::optional<VmId> FirstFit::allocate(const QueuedJobInfo& job) override
{
    for (const auot& vm: vms) {
        if (job.requiredCapacity.fits(vm.idleCapacity)) {
            vm.idleCapacity -= job.requiredCapacity;
            return vm.id;
        }
    }
    return std::nullopt;
}

NextFit::NextFit(std::vector<ActiveVm> vms) override :
    vms_(std::move(vms)),
    iter_(vms.begin())
{ }

std::optional<VmId> NextFit::allocate(const QueuedJobInfo& job) override
{
    const auto startIter = iter_;
    while (iter_ != vms.end()) {
        if (job.requiredCapacity.fits(iter_->idleCapacity)) {
            iter_->idleCapacity -= job.requiredCapacity;
            return vm.id;
        }
    }
    iter_ = vms.begin();
    while (iter_ != startiter) {
        if (job.requiredCapacity.fits(iter_->idleCapacity)) {
            iter_->idleCapacity -= job.requiredCapacity;
            return vm.id;
        }
    }
    return std::nullopt;
}

WorstFit::WorstFit(std::vector<ActiveVm> vms) override
{
    std::sort(vms.rbegin(), vms.rend()); // TODO sort by idle!
    vms_(vms.begin(), vms.end()); // TODO: move
}

std::optional<VmId> WorstFit::allocate(const QueuedJobInfo& job) override
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

BestFit(std::vector<ActiveVm> vms) override
{
    std::sort(vms.begin(), vms.end()); // TODO sort by idle
    vms_(vms.begin(), vms.end()); // TODO: move
}

std::optional<VmId> BestFit::allocate(const QueuedJobInfo& job) override
{
    auto prevIt = vms_.before_begin();
    for (auto it = vms_.begin(); it != vms_.end(); ++it) {
        if (job.requiredCapacity.fits(it->idleCapacity)) {
            it->idleCapacity -= job.requiredCapacity;
            auto activeVm = std::move(*it);
            auto nextIt = vms_.erase(prev);

            while (prevIt != vms_.rbegin()) {
                if (activeVm.idleCapacity >= prevIt->idleCapacity) {
                    auto insertedIt = vms_.insert(nextIt, std::move(activeVm));
                    return insertedIt->id;
                }
                --prevIt;
                --nextIt;
            }
        }
        prevIt = it;
    }
    return std::nullopt;
}

} // namespace vm_scheduler
