#pragma once

#include "lib/task_queue/task_queue.h"

#include <queue>

namespace vm_scheduler::queue {

class InMemoryTaskQueue : public TaskQueue {
public:
    void add(task::Task&& task) override;
    TaskInfos get(const task::VmSpace& vmSpace) override;
private:
//    std::queue<task::Task> queue_;
//    std::vector<task::Task> finished_tasks_;
};

} // namespace vm_scheduler::queue