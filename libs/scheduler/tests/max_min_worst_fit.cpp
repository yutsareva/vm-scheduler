#include "libs/scheduler/impl/complex_vm_assigner.h"
#include "libs/scheduler/tests/test_utils.h"

#include <gtest/gtest.h>

using namespace vm_scheduler;
namespace t = vm_scheduler::testing;

TEST(AscendingWorstFitVmAssigner, assign)
{
    const auto initialState = State{
        .queuedJobs =
            {
                QueuedJobInfo{
                    .id = 0,
                    .requiredCapacity =
                        SlotCapacity{
                            .cpu = 2_cores,
                            .ram = 4096_MB,
                        },
                },
                QueuedJobInfo{
                    .id = 2,
                    .requiredCapacity =
                        SlotCapacity{
                            .cpu = 1_cores,
                            .ram = 1024_MB,
                        },
                },
                QueuedJobInfo{
                    .id = 3,
                    .requiredCapacity =
                        SlotCapacity{
                            .cpu = 6_cores,
                            .ram = 1024_MB,
                        },
                },
                QueuedJobInfo{
                    .id = 4,
                    .requiredCapacity =
                        SlotCapacity{
                            .cpu = 2_cores,
                            .ram = 1024_MB,
                        },
                },
            },
        .vms =
            {
                ActiveVm{
                    .id = 5,
                    .totalCapacity =
                        SlotCapacity{
                            .cpu = 4_cores,
                            .ram = 2048_MB,
                        },
                    .idleCapacity =
                        SlotCapacity{
                            .cpu = 3_cores,
                            .ram = 1024_MB,
                        },
                },
                ActiveVm{
                    .id = 6,
                    .totalCapacity =
                        SlotCapacity{
                            .cpu = 1_cores,
                            .ram = 2048_MB,
                        },
                    .idleCapacity =
                        SlotCapacity{
                            .cpu = 1_cores,
                            .ram = 1024_MB,
                        },
                },
                ActiveVm{
                    .id = 7,
                    .totalCapacity =
                        SlotCapacity{
                            .cpu = 8_cores,
                            .ram = 4096_MB,
                        },
                    .idleCapacity =
                        SlotCapacity{
                            .cpu = 7_cores,
                            .ram = 3584_MB,
                        },
                },
            },
    };
    const auto config = ComplexVmAssignerConfig{
        .jobOrdering = JobOrdering::Ascending,
        .allocationStrategy = AllocationStrategy::WorstFit,
    };
    const auto possibleSlots = t::getPossibleSlots();
    ComplexVmAssigner vmAssigner(config, initialState, possibleSlots);
    const auto stateChange = vmAssigner.assign();

    const auto expectedStateChange = StateChange{
        .jobToVm =
            {
                {
                    4,
                    7,
                },
                {
                    2,
                    7,
                },
                {
                    0,
                    DesiredSlotId(0),
                },
                {
                    3,
                    DesiredSlotId(0),
                },
            },
        .desiredSlotMap =
            {
                {
                    DesiredSlotId(0),
                    DesiredSlot{
                        .total =
                            SlotCapacity{
                                .cpu = 16_cores,
                                .ram = 8192_MB,
                            },
                        .idle =
                            SlotCapacity{
                                .cpu = 8_cores,
                                .ram = 3072_MB,
                            },
                    },
                },
            },
        .updatedIdleCapacities = {
            {
                7,
                {
                    .cpu = 4_cores,
                    .ram = 1536_MB,
                }
            },
        },
        .vmsToTerminate = {},
    };

    EXPECT_EQ(stateChange, expectedStateChange);
    t::checkStateConstrains(initialState, stateChange);
}
