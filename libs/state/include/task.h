#pragma once

#include <libs/common/include/slot.h>
#include <libs/common/include/types.h>

#include <chrono>
#include <unordered_map>


namespace vm_scheduler {

using TaskId = int64_t;

enum class TaskStatus {
    Queued,
    Scheduled,
    Running,
    Completed,
    Error,
    InternalError,
};

struct QueuedTaskInfo {
    TaskId id;
    SlotCapacity requiredCapacity;

    bool operator==(const QueuedTaskInfo&) const = default;
};

struct TaskParameters {
    SlotCapacity requiredCapacity;
    std::chrono::seconds estimation;
    std::string taskInputs;
};

} // namespace vm_scheduler
