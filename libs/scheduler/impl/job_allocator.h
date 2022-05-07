#pragma once

#include "libs/scheduler/include/vm_assigner.h"
#include "libs/scheduler/impl/complex_vm_assigner_config.h"

#include <forward_list>
#include <list>
#include <optional>

namespace vm_scheduler {

class JobAllocator {
public:
    virtual ~JobAllocator() = default;
    virtual std::optional<VmId> allocate(const QueuedJobInfo& job) = 0;
    virtual VmIdToCapacity getVmsWithUpdatedCapacities() = 0;
    virtual std::vector<VmId> getIdleVms() = 0;
};

class FirstFit : public JobAllocator {
public:
    FirstFit(std::vector<ActiveVm> vms);
    std::optional<VmId> allocate(const QueuedJobInfo& job) override;
    VmIdToCapacity getVmsWithUpdatedCapacities() override;
    std::vector<VmId> getIdleVms() override;

private:
    std::vector<ActiveVm> vms_;
    std::unordered_set<VmId> updatedVmIds_;
};

class NextFit : public JobAllocator {
public:
    NextFit(std::vector<ActiveVm> vms);

    std::optional<VmId> allocate(const QueuedJobInfo& job) override;
    VmIdToCapacity getVmsWithUpdatedCapacities() override;
    std::vector<VmId> getIdleVms() override;

private:
    std::vector<ActiveVm> vms_;
    std::vector<ActiveVm>::iterator iter_;
    std::unordered_set<VmId> updatedVmIds_;
};

class WorstFit : public JobAllocator {
public:
    WorstFit(std::vector<ActiveVm> vms);

    std::optional<VmId> allocate(const QueuedJobInfo& job) override;
    VmIdToCapacity getVmsWithUpdatedCapacities() override;
    std::vector<VmId> getIdleVms() override;

private:
    std::forward_list<ActiveVm> vms_;
    std::unordered_set<VmId> updatedVmIds_;
};

class BestFit : public JobAllocator {
public:
    BestFit(std::vector<ActiveVm> vms);

    std::optional<VmId> allocate(const QueuedJobInfo& job) override;
    VmIdToCapacity getVmsWithUpdatedCapacities() override;
    std::vector<VmId> getIdleVms() override;

private:
    std::list<ActiveVm> vms_;
    std::unordered_set<VmId> updatedVmIds_;
};

std::unique_ptr<JobAllocator> createJobAllocator(
    const AllocationStrategy type, std::vector<ActiveVm> vms);

} // namespace vm_scheduler
