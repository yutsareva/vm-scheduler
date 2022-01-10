#include "task_queue.h"

namespace vm_scheduler::queue {

void InMemoryTaskQueue::add(task::Task&& task)
{
    if (isFinal(task.state())) {
        finishedTasks_.emplace_back(std::move(task));
        return;
    }
    queue_.emplace(std::move(task));
}

task::Tasks InMemoryTaskQueue::get(const task::VmSpace& vmSpace, const size_t limit)
{
    task::Tasks tasks;
    tasks.reserve(std::min(queue_.size(), limit));
    for (size_t i = 0 ; i < limit && !queue_.empty(); ++i) {
        tasks.emplace_back(std::move(queue_.front()));
        queue_.pop();
    }
    return tasks;
}

} // namespace vm_scheduler::queue
