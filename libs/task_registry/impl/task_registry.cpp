#include "libs/task_registry/include/task_registry.h"

#include <libs/common/include/log.h>

#include <unistd.h>
#include <string>

namespace vm_scheduler {

TaskRegistry::TaskRegistry(
    const Config& config, std::unique_ptr<TaskStorage>&& taskStorage, std::unique_ptr<CloudClient>&& cloudClient)
    : id_(gethostname())
    , taskStorage_(std::move(taskStorage))
    , allocator_(taskStorage_.get(), std::move(cloudClient))
    , scheduler_(id_, taskStorage_.get())
    , failureDetector_(taskStorage_.get())
    , grpcServer_(createServerConfig(), taskStorage_.get())
    , allocationThread_([this] { allocator_.allocate(); }, config.allocationInterval)
    , terminationThread_([this] { allocator_.terminate(); }, config.allocationInterval)
    , schedulingThread_([this] { scheduler_.schedule(); }, config.schduleInterval)
    , monitorThread_([this] { failureDetector_.monitor(); }, config.detectFailuresInterval)
{
    INFO() << "Backend with id = " << id_ << " started";
}

} // namespace vm_scheduler
