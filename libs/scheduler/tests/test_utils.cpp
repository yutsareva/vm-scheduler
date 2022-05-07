#include "libs/scheduler/tests/test_utils.h"

namespace vm_scheduler::testing {

std::vector<SlotCapacity> getPossibleSlots()
{
    return {
        {
            .cpu = CpuCores(1),
            .ram = MegaBytes(512),
        },
        {
            .cpu = CpuCores(2),
            .ram = MegaBytes(1024),
        },
        {
            .cpu = CpuCores(4),
            .ram = MegaBytes(2048),
        },
        {
            .cpu = CpuCores(8),
            .ram = MegaBytes(4096),
        },
        {
            .cpu = CpuCores(16),
            .ram = MegaBytes(8192),
        },
    };
}

} // namespace vm_scheduler::testing
