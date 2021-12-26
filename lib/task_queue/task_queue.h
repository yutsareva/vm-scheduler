#pragma once

#include "lib/task/task.h"

namespace vm_scheduler::queue {

class TaskQueue {
    virtual void add(Task&&) = 0;
};

} // namespace vm_scheduler::queue
