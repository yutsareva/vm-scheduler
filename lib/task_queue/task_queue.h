#pragma once

#include "task_info.h"

#include <lib/task/task.h>
#include <lib/task/vm_space.h>

namespace vm_scheduler::queue {

class TaskQueue {
    // TODO: rename methods
    virtual void add(task::Task&& task) = 0;
    virtual task::Tasks get(const task::VmSpace& vmSpace, const size_t limit) = 0;
};

} // namespace vm_scheduler::queue
