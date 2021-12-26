#pragma once

#include "task_type.h"
#include "vm_space.h"

namespace vm_scheduler::task {

class Task {
    virtual TaskType type() const = 0;
    //    virtual const Client& client() const;
    virtual TaskState state() const = 0;
    virtual const VmSpace& vmSpace() const = 0;
};

} // namespace vm_scheduler::task
