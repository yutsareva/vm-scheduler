#pragma once

#include <libs/state/include/config.h>

#include <chrono>

namespace vm_scheduler {

struct FailureDetectorConfig {
    std::chrono::seconds allocationTimeLimit;
    std::chrono::seconds agentStartupTimeLimit;
    std::chrono::seconds agentInactivityTimeLimit;
    std::chrono::seconds terminationTimeLimit;
    CommonConfig common;
};

FailureDetectorConfig createFailureDetectorConfig();

} // namespace vm_scheduler
