#include "libs/scheduler/tests/test_utils.h"

namespace vm_scheduler::testing {

std::vector<SlotCapacity> getPossibleSlots()
{
    return {
        {
            .cpu = 1_cores,
            .ram = 512_MB,
        },
        {
            .cpu = 2_cores,
            .ram = 1024_MB,
        },
        {
            .cpu = 4_cores,
            .ram = 2048_MB,
        },
        {
            .cpu = 8_cores,
            .ram = 4096_MB,
        },
        {
            .cpu = 16_cores,
            .ram = 8192_MB,
        },
    };
}

} // namespace vm_scheduler::testing
