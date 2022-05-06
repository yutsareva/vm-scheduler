#include "libs/scheduler/impl/simple_vm_assigner.h"

#include <gtest/gtest.h>

using namespace vm_scheduler;

TEST(SimpleVmAssigner, assign)
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
    SimpleVmAssigner vmAssigner(initialState);
    const auto stateChange = vmAssigner.assign();

//    const auto expectedStateChange = StateChange{
//        .jobToVm =
//            {
//                {
//                    0,
//                    DesiredSlot{
//                        .total =
//                            SlotCapacity{
//                                .cpu = 2_cores,
//                                .ram = 4096_MB,
//                            },
//                        .idle =
//                            SlotCapacity{
//                                .cpu = 0_cores,
//                                .ram = 0_MB,
//                            },
//                    },
//                },
//                {
//                    1,
//                    DesiredSlot{
//                        .total =
//                            SlotCapacity{
//                                .cpu = 1_cores,
//                                .ram = 1024_MB,
//                            },
//                        .idle =
//                            SlotCapacity{
//                                .cpu = 0_cores,
//                                .ram = 0_MB,
//                            },
//                    },
//                },
//            },
//        .vmsToTerminate = {5},
//        .desiredSlotMap = {},
//        .allocatedVmIdToUpdatedIdleCapacity = {},
//    };
//
//    EXPECT_EQ(stateChange, expectedStateChange);
}

TEST(SimpleVmAssigner, emptyJobs)
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
    SimpleVmAssigner vmAssigner(initialState);
    const auto stateChange = vmAssigner.assign();

//    const auto expectedStateChange = StateChange{
//        .vmAssignments = {},
//        .vmsToTerminate = {5, 3},
//        .vmCapacityUpdates = {},
//    };
//
//    EXPECT_EQ(stateChange, expectedStateChange);
}

TEST(SimpleVmAssigner, emptyVms)
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
    SimpleVmAssigner vmAssigner(initialState);
    const auto stateChange = vmAssigner.assign();

//    const auto expectedStateChange = StateChange{
//        .vmAssignments =
//            {
//                {
//                    9,
//                    DesiredSlot{
//                        .total =
//                            SlotCapacity{
//                                .cpu = 3_cores,
//                                .ram = 4096_MB,
//                            },
//                        .idle =
//                            SlotCapacity{
//                                .cpu = 0_cores,
//                                .ram = 0_MB,
//                            },
//                    },
//                },
//            },
//        .vmsToTerminate = {},
//        .vmCapacityUpdates = {},
//    };
//
//    EXPECT_EQ(stateChange, expectedStateChange);
}
