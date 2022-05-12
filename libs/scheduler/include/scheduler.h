#pragma once

#include "libs/scheduler/include/config.h"
#include "libs/zookeeper/include/dist_lock.h"

#include <libs/task_storage/include/task_storage.h>

#include <memory>

namespace vm_scheduler {

class Scheduler {
public:
    Scheduler(
        BackendId id,
        TaskStorage* taskStorage,
        std::vector<SlotCapacity> possibleSlots,
        std::shared_ptr<DistributedLock> distLock);
    void schedule() noexcept;

private:
    BackendId id_;
    TaskStorage* taskStorage_;
    SchedulerConfig config_;
    std::vector<SlotCapacity> possibleSlots_;
    std::shared_ptr<DistributedLock> distLock_;
};

} // namespace vm_scheduler
