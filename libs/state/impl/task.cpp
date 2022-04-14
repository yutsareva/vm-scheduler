#include <libs/state/include/task.h>

#include <iostream>
#include <unordered_map>

namespace vm_scheduler {

const char* toString(JobStatus v)
{
    switch(v) {
        case JobStatus::Queued: return "queued";
        case JobStatus::Scheduled: return "scheduled";
        case JobStatus::Running: return "running";
        case JobStatus::Completed: return "completed";
        case JobStatus::Cancelling: return "cancelling";
        case JobStatus::Cancelled: return "cancelled";
        case JobStatus::Error: return "error";
        case JobStatus::InternalError: return "internal_error";
    }
}

JobStatus jobStatusFromString(const std::string& s)
{
    const static std::unordered_map<std::string, JobStatus> map = {
        {"queued", JobStatus::Queued},
        {"scheduled", JobStatus::Scheduled},
        {"running", JobStatus::Running},
        {"completed", JobStatus::Completed},
        {"cancelling", JobStatus::Cancelling},
        {"cancelled", JobStatus::Cancelled},
        {"error", JobStatus::Error},
        {"internal_error", JobStatus::InternalError},
    };
    return map.at(s);
}

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
