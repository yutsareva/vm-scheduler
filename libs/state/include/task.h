#pragma once

#include <libs/common/include/slot.h>
#include <libs/common/include/types.h>

#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace vm_scheduler {

using JobId = int64_t;

enum class JobStatus {
    Queued /* "queued" */,
    Scheduled /* "scheduled" */,
    Running /* "running" */,
    Completed /* "completed" */,
    Cancelling /* "cancelling" */,
    Cancelled /* "cancelled" */,
    Error /* "error" */,
    InternalError /* "internal_error" */,
};

const std::unordered_set<JobStatus>& getFinalJobStatuses();

const std::unordered_set<JobStatus>& getAssignedJobStatuses();

struct QueuedJobInfo {
    JobId id;
    SlotCapacity requiredCapacity;

    bool operator==(const QueuedJobInfo&) const = default;
};

std::ostream& operator<<(std::ostream& out, const std::vector<QueuedJobInfo>& jobInfos);

using TaskSettings = std::string;
using JobOptions = std::string;
using ImageVersion = std::string;
using ClientId = std::string;

struct TaskParameters {
    SlotCapacity requiredCapacity;
    std::chrono::seconds estimation;
    size_t jobCount;
    TaskSettings settings;
    ImageVersion imageVersion;
    ClientId clientId;

    std::vector<JobOptions> jobOptions;
};

using TaskId = int64_t;

struct CreatedJobs {
    TaskId taskId;
    std::vector<JobId> jobIds;

    bool operator==(const CreatedJobs&) const = default;
};

using JobResultUrl = std::string;

struct JobState {
    JobStatus status;
    std::optional<JobResultUrl> resultUrl;
};

using JobStates = std::vector<JobState>;

struct JobToLaunch {
    JobId id;
    SlotCapacity capacityLimits;
    TaskSettings taskSettings;
    ImageVersion imageVersion;
    JobOptions jobOptions;
};

struct AssignedJob {
    JobId id;
    JobStatus status;
};

} // namespace vm_scheduler
