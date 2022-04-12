#pragma once

#include "libs/allocator/include/cloud_client.h"
#include "libs/allocator/include/config.h"

#include <libs/task_storage/include/task_storage.h>

namespace vm_scheduler {

class Allocator {
public:
    Allocator(TaskStorage* taskStorage, std::unique_ptr<CloudClient>&& cloudClient);
    void allocate() noexcept;
    void terminate() noexcept;

private:
    TaskStorage* taskStorage_;
    std::unique_ptr<CloudClient> cloudClient_;
    AllocatorConfig config_;
};

} // namespace vm_scheduler
