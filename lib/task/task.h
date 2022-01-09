#pragma once

#include "task_state.h"
#include "task_type.h"
#include "vm_space.h"

namespace vm_scheduler::task {

class Task {
public:
    TaskType type() const { return type_; }
    TaskState state() const { return state_; }
    const VmSpace& vmSpace() const { return vmSpace_; }

private:
    TaskType type_;
    TaskState state_;
    VmSpace vmSpace_;
};

using Tasks = std::vector<Task>;

} // namespace vm_scheduler::task
