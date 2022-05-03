#pragma once

#include "libs/scheduler/include/vm_assigner.h"

#include <optional>

namespace vm_scheduler {

class JobAllocator {
public:
    virtual JobAllocator(std::vector<ActiveVm> vms) = 0;
    virtual ~JobAllocator = default;
    virtual std::optional<VmId> allocate(const QueuedJobInfo& job) = 0;
};

class FirstFit : public JobAllocator {
public:
    FirstFit(std::vector<ActiveVm> vms) override;
    std::optional<VmId> allocate(const QueuedJobInfo& job) override;

private:
    std::vector<ActiveVm> vms_;
};

class NextFit : public JobAllocator {
public:
    NextFit(std::vector<ActiveVm> vms) override;

    std::optional<VmId> allocate(const QueuedJobInfo& job) override;

private:
    std::vector<ActiveVm> vms_;
    std::vector<ActiveVm>::iterator iter_;
};

class WorstFit : public JobAllocator {
public:
    WorstFit(std::vector<ActiveVm> vms) override;

    std::optional<VmId> allocate(const QueuedJobInfo& job) override;

private:
    std::forward_list<ActiveVm> vms_;
};

class BestFit : public JobAllocator {
public:
    BestFit(std::vector<ActiveVm> vms) override;

    std::optional<VmId> allocate(const QueuedJobInfo& job) override;

private:
    std::list<ActiveVm> vms_;
};

} // namespace vm_scheduler
