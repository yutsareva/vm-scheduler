#include "libs/scheduler/impl/job_allocator.h"

namespace vm_scheduler {

FirstFit::FirstFit(std::vector<ActiveVm> vms) : vms_(std::move(vms)) { }

std::optional<VmId> FirstFit::allocate(const QueuedJobInfo& job)
{
    for (auto& vm: vms_) {
        if (job.requiredCapacity.fits(vm.idleCapacity)) {
            vm.idleCapacity -= job.requiredCapacity;
            updatedVmIds_.insert(vm.id);
            return vm.id;
        }
    }
    return std::nullopt;
}

VmIdToCapacity FirstFit::getVmsWithUpdatedCapacities()
{
    VmIdToCapacity updatedVmsCapacities;
    for (const auto& vm: vms_) {
        if (updatedVmIds_.contains(vm.id)) {
            updatedVmsCapacities.emplace(vm.id, vm.idleCapacity);
        }
    }
    return updatedVmsCapacities;
}

std::vector<VmId> FirstFit::getIdleVms()
{
    std::vector<VmId> idleVms;
    for (const auto& vm: vms_) {
        if (vm.totalCapacity == vm.idleCapacity) {
            idleVms.push_back(vm.id);
        }
    }
    return idleVms;
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
            updatedVmIds_.insert(iter_->id);
            return iter_->id;
        }
    }
    iter_ = vms_.begin();
    while (iter_ != startIter) {
        if (job.requiredCapacity.fits(iter_->idleCapacity)) {
            iter_->idleCapacity -= job.requiredCapacity;
            updatedVmIds_.insert(iter_->id);
            return iter_->id;
        }
    }
    return std::nullopt;
}

VmIdToCapacity NextFit::getVmsWithUpdatedCapacities()
{
    VmIdToCapacity updatedVmsCapacities;
    for (const auto& vm: vms_) {
        if (updatedVmIds_.contains(vm.id)) {
            updatedVmsCapacities.emplace(vm.id, vm.idleCapacity);
        }
    }
    return updatedVmsCapacities;
}

std::vector<VmId> NextFit::getIdleVms()
{
    std::vector<VmId> idleVms;
    for (const auto& vm: vms_) {
        if (vm.totalCapacity == vm.idleCapacity) {
            idleVms.push_back(vm.id);
        }
    }
    return idleVms;
}

WorstFit::WorstFit(std::vector<ActiveVm> vms)
{
    std::sort(vms.begin(), vms.end(), [](const ActiveVm& lhs, const ActiveVm& rhs) {
        return rhs.idleCapacity < lhs.idleCapacity;
    });
    vms_ = {
        std::make_move_iterator(vms.begin()), std::make_move_iterator(vms.end())};
}

std::optional<VmId> WorstFit::allocate(const QueuedJobInfo& job)
{
    if (vms_.empty()) {
        return std::nullopt;
    }
    auto it = vms_.end();
    --it;
    if (vms_.empty() || !job.requiredCapacity.fits(it->idleCapacity)) {
        return std::nullopt;
    }
    auto updatedActiveVm = *it;
    updatedActiveVm.idleCapacity -= job.requiredCapacity;

    vms_.erase(it);
    updatedVmIds_.insert(updatedActiveVm.id);
    const auto inserted = vms_.insert(std::move(updatedActiveVm));

    return inserted->id;
}

VmIdToCapacity WorstFit::getVmsWithUpdatedCapacities()
{
    VmIdToCapacity updatedVmsCapacities;
    for (const auto& vm: vms_) {
        if (updatedVmIds_.contains(vm.id)) {
            updatedVmsCapacities.emplace(vm.id, vm.idleCapacity);
        }
    }
    return updatedVmsCapacities;
}

std::vector<VmId> WorstFit::getIdleVms()
{
    std::vector<VmId> idleVms;
    for (const auto& vm: vms_) {
        if (vm.totalCapacity == vm.idleCapacity) {
            idleVms.push_back(vm.id);
        }
    }
    return idleVms;
}

BestFit::BestFit(std::vector<ActiveVm> vms)
{
    std::sort(vms.begin(), vms.end(), [](const ActiveVm& lhs, const ActiveVm& rhs) {
        return lhs.idleCapacity < rhs.idleCapacity;
    });
    vms_ = {
        std::make_move_iterator(vms.begin()), std::make_move_iterator(vms.end())};
}

std::optional<VmId> BestFit::allocate(const QueuedJobInfo& job)
{
    // binary search using non-linear order via fits function
    auto it = std::lower_bound(
        vms_.begin(),
        vms_.end(),
        job.requiredCapacity,
        [](const ActiveVm& vm, const SlotCapacity& slot) {
            return !slot.fits(vm.idleCapacity);
        });

    if (it == vms_.end() || !job.requiredCapacity.fits(it->idleCapacity)) {
        return std::nullopt;
    }
    auto updatedActiveVm = *it;
    updatedActiveVm.idleCapacity -= job.requiredCapacity;

    vms_.erase(it);
    updatedVmIds_.insert(updatedActiveVm.id);
    const auto inserted = vms_.insert(std::move(updatedActiveVm));

    return inserted->id;
}

VmIdToCapacity BestFit::getVmsWithUpdatedCapacities()
{
    VmIdToCapacity updatedVmsCapacities;
    for (const auto& vm: vms_) {
        if (updatedVmIds_.contains(vm.id)) {
            updatedVmsCapacities.emplace(vm.id, vm.idleCapacity);
        }
    }
    return updatedVmsCapacities;
}

std::vector<VmId> BestFit::getIdleVms()
{
    std::vector<VmId> idleVms;
    for (const auto& vm: vms_) {
        if (vm.totalCapacity == vm.idleCapacity) {
            idleVms.push_back(vm.id);
        }
    }
    return idleVms;
}

std::unique_ptr<JobAllocator> createJobAllocator(
    const AllocationStrategy type, std::vector<ActiveVm> vms)
{
    switch (type) {
        case AllocationStrategy::FirstFit:
            return std::make_unique<FirstFit>(std::move(vms));
        case AllocationStrategy::NextFit:
            return std::make_unique<NextFit>(std::move(vms));
        case AllocationStrategy::WorstFit:
            return std::make_unique<WorstFit>(std::move(vms));
        case AllocationStrategy::BestFit:
            return std::make_unique<BestFit>(std::move(vms));
    }
}

} // namespace vm_scheduler
