#include "libs/scheduler/impl/ordered_jobs.h"

namespace vm_scheduler {

FifoOrderedJobs::FifoOrderedJobs(std::vector<QueuedJobInfo>&& jobs)
    : OrderedJobs(std::move(jobs))
{ }

MinMinOrderedJobs::MinMinOrderedJobs(std::vector<QueuedJobInfo>&& jobs)
    : OrderedJobs(std::move(jobs))
{
    std::sort(
        jobs_.begin(),
        jobs_.end(),
        [](const QueuedJobInfo& lhs, const QueuedJobInfo& rhs) {
            return lhs.requiredCapacity < rhs.requiredCapacity;
        });
}

MaxMinOrderedJobs::MaxMinOrderedJobs(std::vector<QueuedJobInfo>&& jobs)
    : OrderedJobs(std::move(jobs))
{
    std::sort(
        jobs_.rbegin(),
        jobs_.rend(),
        [](const QueuedJobInfo& lhs, const QueuedJobInfo& rhs) {
            return lhs.requiredCapacity < rhs.requiredCapacity;
        });
}

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
