#pragma once

#include "lib/task/task.h"
#include "lib/task_queue/task_queue.h"

#include <queue>

namespace vm_scheduler::queue {

class InMemoryTaskQueue : public TaskQueue {
public:
    void add(task::Task&& task) override;
    task::Tasks get(const task::VmSpace& vmSpace, const size_t limit) override;
private:
    std::queue<task::Task> queue_;
    std::vector<task::Task> finishedTasks_;
};

} // namespace vm_scheduler::queue
