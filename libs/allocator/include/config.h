#pragma once

#include <libs/state/include/config.h>

#include <string>

namespace vm_scheduler {

struct AllocatorConfig {
    size_t maxVmAllocationCount;
    size_t maxVmTerminationCount;
    CommonConfig common;
};

AllocatorConfig createAllocatorConfig();

} // namespace vm_scheduler
