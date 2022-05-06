#pragma once

#include "libs/state/include/task.h"

namespace vm_scheduler {

class OrderedJobs {
public:
    OrderedJobs(std::vector<QueuedJobInfo> jobs)
        : jobs_(std::move(jobs))
    {}
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
    FifoOrderedJobs(std::vector<QueuedJobInfo>&& jobs) :
        OrderedJobs(std::move(jobs))
    {
        std::sort(jobs_.begin(), jobs_.end());
    }
};

class MinMinOrderedJobs : public OrderedJobs {
public:
    MinMinOrderedJobs(std::vector<QueuedJobInfo>&& jobs) :
        OrderedJobs(std::move(jobs))
    {
        std::sort(
            jobs_.begin(),
            jobs_.end(),
            [](const QueuedJobInfo& first, const QueuedJobInfo& second) {
                return first.requiredCapacity < second.requiredCapacity;
            });
    }
};

class MaxMinOrderedJobs : public OrderedJobs {
public:
    MaxMinOrderedJobs(std::vector<QueuedJobInfo>&& jobs) :
        OrderedJobs(std::move(jobs))
    {
        std::sort(
            jobs_.rbegin(),
            jobs_.rend(),
            [](const QueuedJobInfo& first, const QueuedJobInfo& second) {
                return first.requiredCapacity < second.requiredCapacity;
            });
    }
};

std::unique_ptr<OrderedJobs> createOrderedJobs(
    const JobOrdering type, std::vector<QueuedJobInfo>&& jobs)
{
    switch (type) {
        case JobOrdering::Fifo:
            return std::make_unique<FifoOrderedJobs>(std::move(jobs));
        case JobOrdering::MinMin:
            return std::make_unique<MinMinOrderedJobs>(std::move(jobs));
        case JobOrdering::MaxMin:
            return std::make_unique<MaxMinOrderedJobs>(std::move(jobs));
    }
}

} // namespace vm_scheduler
