#pragma once

namespace vm_scheduler::task {

enum class TaskState {
    Created,
    Started,
    Completed,
};

bool isFinal(const TaskState state)
{
    return state == TaskState::Completed;
}

} // namespace vm_scheduler::task
