#include "libs/task_registry/include/task_registry.h"
#include "libs/zookeeper/include/dist_lock.h"

#include <libs/common/include/log.h>

//#include <unistd.h>
#include <string>

namespace vm_scheduler {

TaskRegistry::TaskRegistry(
    const Config& config,
    std::unique_ptr<TaskStorage>&& taskStorage,
    std::unique_ptr<CloudClient>&& cloudClient)
    : id_("1234") // TODO: hostname
    , taskStorage_(std::move(taskStorage))
    , scheduler_(
          id_,
          taskStorage_.get(),
          cloudClient->getPossibleSlots(),
          createDistLock(
              config.useZkDistLock &&
              (config.mode == SchedulerMode::FullScheduler ||
               config.mode == SchedulerMode::CoreScheduler)))
    , allocator_(taskStorage_.get(), std::move(cloudClient))
    , failureDetector_(taskStorage_.get(), &allocator_)
    , grpcServer_(createServerConfig(), taskStorage_.get())
{
    if (config.mode == SchedulerMode::FullScheduler ||
        config.mode == SchedulerMode::SchedulerService) {
        allocationThread_ = std::make_unique<BackgroundThread>(
            [this] { allocator_.allocate(); }, config.allocationInterval);
        terminationThread_ = std::make_unique<BackgroundThread>(
            [this] { allocator_.terminate(); }, config.allocationInterval);
        monitorThread_ = std::make_unique<BackgroundThread>(
            [this] { failureDetector_.monitor(); },
            config.detectFailuresInterval);
    }

    if (config.mode == SchedulerMode::FullScheduler ||
        config.mode == SchedulerMode::CoreScheduler) {
        schedulingThread_ = std::make_unique<BackgroundThread>(
            [this] { scheduler_.schedule(); }, config.scheduleInterval);
    }

    INFO() << "Backend with id = " << id_ << " started";
}

} // namespace vm_scheduler
