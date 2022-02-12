#pragma once

#include "libs/task_registry/include/config.h"

#include <libs/allocator/include/allocator.h>
#include <libs/task_storage/include/task_storage.h>
#include <libs/common/include/types.h>
#include <libs/scheduler/include/scheduler.h>

#include <memory>
#include <string>


namespace vm_scheduler {

class TaskRegistry {
public:
    TaskRegistry(
        const Config& config,
        std::unique_ptr<TaskStorage>&& taskStorage,
        std::unique_ptr<CloudClient>&& cloudClient);

private:
    BackendId id_;

    std::unique_ptr<TaskStorage> taskStorage_;
    Allocator allocator_;
    Scheduler scheduler_;

    // TODO: background threads: allocate_, schedule_
};

} // namespace vm_scheduler
