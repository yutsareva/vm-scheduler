#pragma once

#include "lib/task/task.h"
#include "lib/task/vm_space.h"

namespace vm_scheduler::queue {

class TaskQueue {
    virtual void add(task::Task&& task) = 0;
    virtual TaskInfos get(const task::VmSpace& vmSpace) = 0;
};

} // namespace vm_scheduler::queue
