#include "task_queue.h"

namespace vm_scheduler::queue {

void InMemoryTaskQueue::add(task::Task&& task)
{
    if (isFinal(task.state())) {
        finishedTasks_.emplace(std::move(task));
        return;
    }
    queue_.emplace(std::move(task));
}

Tasks InMemoryTaskQueue::get(const task::VmSpace& vmSpace, const size_t limit)
{
    Tasks tasks;
    tasks.reserve(std::min(queue_.size(), count));
    for (size_t i = 0 ; i < count && !queue_.enplty(); ++i) {
        tasks.emplace(queue_.pop());
    }
    return tasks;
}

} // namespace vm_scheduler::queue