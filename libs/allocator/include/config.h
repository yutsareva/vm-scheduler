#pragma once

#include <string>


namespace vm_scheduler {

struct AllocatorConfig {
    size_t maxVmAllocationCount;
    size_t maxVmTerminationCount;
};

AllocatorConfig createAllocatorConfig();

} // namespace vm_scheduler
