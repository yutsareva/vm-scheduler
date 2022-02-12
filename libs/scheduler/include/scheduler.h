#pragma once

#include "libs/scheduler/include/config.h"

#include <libs/task_storage/include/task_storage.h>


namespace vm_scheduler {

class Scheduler {
public:
    Scheduler(BackendId id, TaskStorage* taskStorage);
    void schedule() noexcept;

private:
    BackendId id_;
    TaskStorage* taskStorage_;
    SchedulerConfig config_;
};

} // namespace vm_scheduler
