#include "libs/scheduler/impl/complex_vm_assigner.h"
#include "libs/scheduler/tests/test_utils.h"

#include <gtest/gtest.h>

using namespace vm_scheduler;
namespace t = vm_scheduler::testing;

TEST(MaxMinBestFitVmAssigner, assign)
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
                    .id = 1,
                    .requiredCapacity =
                        SlotCapacity{
                            .cpu = 1_cores,
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
                            .cpu = 4_cores,
                            .ram = 2048_MB,
                        },
                },
            },
    };
    const auto config = ComplexVmAssignerConfig{
        .jobOrdering = JobOrdering::MaxMin,
        .allocationStrategy = AllocationStrategy::BestFit,
    };
    const auto possibleSlots = t::getPossibleSlots();
    ComplexVmAssigner vmAssigner(config, initialState, possibleSlots);
    const auto stateChange = vmAssigner.assign();

    const auto expectedStateChange = StateChange{
        .jobToVm =
            {
                {
                    0,
                    DesiredSlotId(0),
                },
                {
                    1,
                    5,
                },
            },
        .desiredSlotMap =
            {
                {
                    DesiredSlotId(0),
                    DesiredSlot{
                        .total =
                            SlotCapacity{
                                .cpu = 8_cores,
                                .ram = 4096_MB,
                            },
                        .idle =
                            SlotCapacity{
                                .cpu = 6_cores,
                                .ram = 0_MB,
                            },
                    },
                },
            },
        .updatedIdleCapacities = {
            {
                5,
                {
                    .cpu = 3_cores,
                    .ram = 1024_MB,
                }
            }
        },
        .vmsToTerminate = {},
    };

    EXPECT_EQ(stateChange, expectedStateChange);
    t::checkStateConstrains(initialState, stateChange);
}

TEST(MaxMinBestFitVmAssigner, emptyJobs)
{
    const auto initialState = State{
        .queuedJobs = {},
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
                            .cpu = 4_cores,
                            .ram = 2048_MB,
                        },
                },
                ActiveVm{
                    .id = 1,
                    .totalCapacity =
                        SlotCapacity{
                            .cpu = 2_cores,
                            .ram = 2048_MB,
                        },
                    .idleCapacity =
                        SlotCapacity{
                            .cpu = 1_cores,
                            .ram = 1024_MB,
                        },
                },
                ActiveVm{
                    .id = 3,
                    .totalCapacity =
                        SlotCapacity{
                            .cpu = 2_cores,
                            .ram = 1024_MB,
                        },
                    .idleCapacity =
                        SlotCapacity{
                            .cpu = 2_cores,
                            .ram = 1024_MB,
                        },
                },
            },
    };

    const auto config = ComplexVmAssignerConfig{
        .jobOrdering = JobOrdering::MaxMin,
        .allocationStrategy = AllocationStrategy::BestFit,
    };
    const auto possibleSlots = t::getPossibleSlots();
    ComplexVmAssigner vmAssigner(config, initialState, possibleSlots);
    const auto stateChange = vmAssigner.assign();

        const auto expectedStateChange = StateChange{
            .jobToVm = {},
            .desiredSlotMap = {},
            .updatedIdleCapacities = {},
            .vmsToTerminate = {3, 5},
        };

    EXPECT_EQ(stateChange, expectedStateChange);
    t::checkStateConstrains(initialState, stateChange);
}

TEST(MaxMinBestFitVmAssigner, emptyVms)
{
    const auto initialState = State{
        .queuedJobs =
            {
                QueuedJobInfo{
                    .id = 9,
                    .requiredCapacity =
                        SlotCapacity{
                            .cpu = 3_cores,
                            .ram = 4096_MB,
                        },
                },
            },
        .vms = {},
    };
    const auto config = ComplexVmAssignerConfig{
        .jobOrdering = JobOrdering::MaxMin,
        .allocationStrategy = AllocationStrategy::BestFit,
    };
    const auto possibleSlots = t::getPossibleSlots();
    ComplexVmAssigner vmAssigner(config, initialState, possibleSlots);
    const auto stateChange = vmAssigner.assign();


    const auto expectedStateChange = StateChange{
        .jobToVm =
            {
                {
                    9,
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
                                .cpu = 8_cores,
                                .ram = 4096_MB,
                            },
                        .idle =
                            SlotCapacity{
                                .cpu = 5_cores,
                                .ram = 0_MB,
                            },
                    },
                },
            },
        .updatedIdleCapacities = {},
        .vmsToTerminate = {},
    };

    EXPECT_EQ(stateChange, expectedStateChange);
    t::checkStateConstrains(initialState, stateChange);
}

TEST(MaxMinBestFitVmAssigner, assignTest2)
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
            },
    };
    const auto config = ComplexVmAssignerConfig{
        .jobOrdering = JobOrdering::MaxMin,
        .allocationStrategy = AllocationStrategy::BestFit,
    };
    const auto possibleSlots = t::getPossibleSlots();
    ComplexVmAssigner vmAssigner(config, initialState, possibleSlots);
    const auto stateChange = vmAssigner.assign();

    const auto expectedStateChange = StateChange{
        .jobToVm =
            {
                {
                    4,
                    5,
                },
                {
                    2,
                    6,
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
                5,
                {
                    .cpu = 1_cores,
                    .ram = 0_MB,
                }
            },
            {
                6,
                {
                    .cpu = 0_cores,
                    .ram = 0_MB,
                }
            }
        },
        .vmsToTerminate = {},
    };

    EXPECT_EQ(stateChange, expectedStateChange);
    t::checkStateConstrains(initialState, stateChange);
}


TEST(MaxMinBestFitVmAssigner, assignTest3)
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
        .jobOrdering = JobOrdering::MaxMin,
        .allocationStrategy = AllocationStrategy::BestFit,
    };
    const auto possibleSlots = t::getPossibleSlots();
    ComplexVmAssigner vmAssigner(config, initialState, possibleSlots);
    const auto stateChange = vmAssigner.assign();

    const auto expectedStateChange = StateChange{
        .jobToVm =
            {
                {
                    4,
                    5,
                },
                {
                    2,
                    6,
                },
                {
                    0,
                    DesiredSlotId(0),
                },
                {
                    3,
                    7,
                },
            },
        .desiredSlotMap =
            {
                {
                    DesiredSlotId(0),
                    DesiredSlot{
                        .total =
                            SlotCapacity{
                                .cpu = 8_cores,
                                .ram = 4096_MB,
                            },
                        .idle =
                            SlotCapacity{
                                .cpu = 6_cores,
                                .ram = 0_MB,
                            },
                    },
                },
            },
        .updatedIdleCapacities = {
            {
                7,
                {
                    .cpu = 1_cores,
                    .ram = 2560_MB,
                }
            },
            {
                5,
                {
                    .cpu = 1_cores,
                    .ram = 0_MB,
                }
            },
            {
                6,
                {
                    .cpu = 0_cores,
                    .ram = 0_MB,
                }
            },
        },
        .vmsToTerminate = {},
    };

    EXPECT_EQ(stateChange, expectedStateChange);
    t::checkStateConstrains(initialState, stateChange);
}
