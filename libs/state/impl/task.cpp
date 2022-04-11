#include <libs/state/include/task.h>

#include <iostream>


namespace vm_scheduler {

const std::unordered_set<JobStatus>& getFinalJobStatuses()
{
    static const std::unordered_set<JobStatus> finalJobStatuses = {
        JobStatus::Completed,
        JobStatus::Cancelled,
        JobStatus::Error,
        JobStatus::InternalError,
    };
    return finalJobStatuses;
}

std::ostream& operator<<(std::ostream& out, const std::vector<QueuedJobInfo>& jobInfos)
{
    for (const auto& jobInfo : jobInfos) {
        out << "[id: " << jobInfo.id << ", "
            << "slot capacity: " << jobInfo.requiredCapacity << "], ";
    }
    return out;
}

const std::unordered_set<JobStatus>& getAssignedJobStatuses()
{
    static const std::unordered_set<JobStatus> assignedJobStatuses = {
        JobStatus::Scheduled,
        JobStatus::Running,
        JobStatus::Cancelling,
    };
    return assignedJobStatuses;
}

} // namespace vm_scheduler
