#include "libs/allocator/include/config.h"

#include <libs/common/include/env.h>

namespace vm_scheduler {

namespace {

constexpr size_t DEFAULT_MAX_VM_ALLOCATION_COUNT{1};
constexpr size_t DEFAULT_MAX_VM_TERMINATION_COUNT{1};

} // anonymous namespace

AllocatorConfig createAllocatorConfig()
{
    return {
        .maxVmAllocationCount = getFromEnvOrDefault("VMS_MAX_VM_ALLOCATION_COUNT", DEFAULT_MAX_VM_ALLOCATION_COUNT),
        .maxVmTerminationCount = getFromEnvOrDefault("VMS_MAX_VM_TERMINATION_COUNT", DEFAULT_MAX_VM_TERMINATION_COUNT),
        .common = createCommonConfig(),
    };
}

} // namespace vm_scheduler
