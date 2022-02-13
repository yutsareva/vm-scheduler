#include "libs/task_registry/include/task_registry.h"

#include <libs/common/include/log.h>

#include <string>


namespace vm_scheduler {

TaskRegistry::TaskRegistry(
    const Config& config, std::unique_ptr<TaskStorage>&& taskStorage, std::unique_ptr<CloudClient>&& cloudClient)
    : id_("TODO")
    , taskStorage_(std::move(taskStorage))
    , allocator_(taskStorage_.get(), std::move(cloudClient))
    , scheduler_(id_, taskStorage_.get())
{
    INFO() << "Backend with id = " << id_ << " started";
}

} // namespace vm_scheduler


// TODO: tests
