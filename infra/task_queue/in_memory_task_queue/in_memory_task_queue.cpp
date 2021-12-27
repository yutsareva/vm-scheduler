#include "in_memory_task_queue.h"

namespace vm_scheduler::queue {

void InMemoryTaskQueue::add(task::Task&& task)
{
//    queue_.emplace(std::move(task));
}

TaskInfos InMemoryTaskQueue::get(const task::VmSpace& vmSpace)
{
    return TaskInfos();
}

} // namespace vm_scheduler::queue