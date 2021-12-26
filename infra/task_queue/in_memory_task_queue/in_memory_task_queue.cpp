#include "in_memory_task_queue.h"

namespace vm_scheduler::queue {

void InMemoryTaskQueue::add(task::Task&& task) override
{
    queue_.push_back(std::move(task));
}

TaskInfos InMemoryTaskQueue::get(const task::VmSpace& vmSpace) override
{

}

} // namespace vm_scheduler::queue