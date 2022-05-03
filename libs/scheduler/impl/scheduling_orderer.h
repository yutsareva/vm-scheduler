#pragma once

#include "libs/scheduler/include/vm_assigner.h"

namespace vm_scheduler {

class SchedulingOrderer {
public:
    static virtual void reorder(std::vector<QueuedJobInfo>& queuedJobs) = 0;
};

class FifoOrderer : public SchedulingOrderer {
public:
    static void reorder(std::vector<QueuedJobInfo>& queuedJobs) override
    {
        // TODO check order
        return;
    }
};

class MinMinOrderer : public SchedulingOrderer {
public:
    static void reorder(std::vector<QueuedJobInfo>& queuedJobs) override
    {
        std::sort(
            queuedJobs.begin(),
            queuedJobs.end(),
            [](const QueuedJobInfo& first, const QueuedJobInfo& second) {
                return first.capacity < second.capacity;
            });
    }
};

class MaxMinOrderer : public SchedulingOrderer {
public:
    static void reorder(std::vector<QueuedJobInfo>& queuedJobs) override
    {
        std::sort(
            queuedJobs.rbegin(),
            queuedJobs.rend(),
            [](const QueuedJobInfo& first, const QueuedJobInfo& second) {
                return first.capacity < second.capacity;
            });
    }
};

std::unique_ptr<SchedulingOrderer> createSchedulingOrderer(const OrdererType type)
{
    switch (type) {
        case OrdererType::Fifo:
            return std::make_unique<FifoOrderer>();
        case OrdererType::MinMin:
            return std::make_unique<MinMinOrderer>();
        case OrdererType::MaxMin:
            return std::make_unique<MaxMinOrderer>();
    }
}

} // namespace vm_scheduler
