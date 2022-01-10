#pragma once

#include "task_state.h"
#include "task_type.h"
#include "vm_space.h"

#include <lib/common/slot.h>

#include <string>
#include <vector>

namespace vm_scheduler::task {

using TaskId = std::string;

struct TaskSlot {
    TaskId taskId;
    SlotCapacity capacity;
};

class Task {
public:
    const TaskId& id() const { return id_; }
    TaskType type() const { return type_; }
    TaskState state() const { return state_; }
    const VmSpace& vmSpace() const { return vmSpace_; }
    TaskSlot taskSlot() const { return {.taskId = id_, .capacity = capacity_}; }

private:
    TaskId id_;
    TaskType type_;
    TaskState state_;
    VmSpace vmSpace_;
    SlotCapacity capacity_;
};

using Tasks = std::vector<Task>;

} // namespace vm_scheduler::task
