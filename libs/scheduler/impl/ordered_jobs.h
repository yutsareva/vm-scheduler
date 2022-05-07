#pragma once

#include "libs/scheduler/impl/complex_vm_assigner_config.h"

#include <libs/state/include/task.h>

#include <memory>

namespace vm_scheduler {

class OrderedJobs {
public:
    OrderedJobs(std::vector<QueuedJobInfo> jobs) : jobs_(std::move(jobs)) { }
    using iterator = std::vector<QueuedJobInfo>::iterator;
    using const_iterator = std::vector<QueuedJobInfo>::const_iterator;

    iterator begin() { return jobs_.begin(); }
    const_iterator begin() const { return jobs_.begin(); }
    iterator end() { return jobs_.end(); }
    const_iterator end() const { return jobs_.end(); }

protected:
    std::vector<QueuedJobInfo> jobs_;
};

class FifoOrderedJobs : public OrderedJobs {
public:
    FifoOrderedJobs(std::vector<QueuedJobInfo>&& jobs);
};

class MinMinOrderedJobs : public OrderedJobs {
public:
    MinMinOrderedJobs(std::vector<QueuedJobInfo>&& jobs);
};

class MaxMinOrderedJobs : public OrderedJobs {
public:
    MaxMinOrderedJobs(std::vector<QueuedJobInfo>&& jobs);
};

std::unique_ptr<OrderedJobs> createOrderedJobs(
    const JobOrdering type, std::vector<QueuedJobInfo>&& jobs);

} // namespace vm_scheduler
