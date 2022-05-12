#include "libs/scheduler/impl/ordered_jobs.h"

namespace vm_scheduler {

FifoOrderedJobs::FifoOrderedJobs(std::vector<QueuedJobInfo>&& jobs)
    : OrderedJobs(std::move(jobs))
{ }

AscendingOrderedJobs::AscendingOrderedJobs(std::vector<QueuedJobInfo>&& jobs)
    : OrderedJobs(std::move(jobs))
{
    std::sort(
        jobs_.begin(),
        jobs_.end(),
        [](const QueuedJobInfo& lhs, const QueuedJobInfo& rhs) {
            return lhs.requiredCapacity < rhs.requiredCapacity;
        });
}

DescendingOrderedJobs::DescendingOrderedJobs(std::vector<QueuedJobInfo>&& jobs)
    : OrderedJobs(std::move(jobs))
{
    std::sort(
        jobs_.begin(),
        jobs_.end(),
        [](const QueuedJobInfo& lhs, const QueuedJobInfo& rhs) {
            return lhs.requiredCapacity > rhs.requiredCapacity;
        });
}

std::unique_ptr<OrderedJobs> createOrderedJobs(
    const JobOrdering type, std::vector<QueuedJobInfo>&& jobs)
{
    switch (type) {
        case JobOrdering::Fifo:
            return std::make_unique<FifoOrderedJobs>(std::move(jobs));
        case JobOrdering::Ascending:
            return std::make_unique<AscendingOrderedJobs>(std::move(jobs));
        case JobOrdering::Descending:
            return std::make_unique<DescendingOrderedJobs>(std::move(jobs));
    }
}

} // namespace vm_scheduler
