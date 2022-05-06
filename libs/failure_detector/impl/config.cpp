#include "libs/state/include/config.h"
#include <libs/failure_detector/include/config.h>

#include <libs/common/include/env.h>

namespace vm_scheduler {

namespace {

constexpr size_t DEFAULT_ALLOCATION_TIME_LIMIT_S{30};
constexpr size_t DEFAULT_AGENT_STARTUP_TIME_LIMIT_S{300};
constexpr size_t DEFAULT_AGENT_INACTIVITY_TIME_LIMIT_S{120};
constexpr size_t DEFAULT_TERMINATION_TIME_LIMIT_S{120};

} // anonymous namespace

FailureDetectorConfig createFailureDetectorConfig()
{
    return {
        .allocationTimeLimit = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_ALLOCATION_TIME_LIMIT_S", DEFAULT_ALLOCATION_TIME_LIMIT_S)},
        .agentStartupTimeLimit = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_AGENT_STARTUP_TIME_LIMIT_S", DEFAULT_AGENT_STARTUP_TIME_LIMIT_S)},
        .agentInactivityTimeLimit = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_AGENT_INACTIVITY_TIME_LIMIT_S",
            DEFAULT_AGENT_INACTIVITY_TIME_LIMIT_S)},
        .terminationTimeLimit = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_TERMINATION_TIME_LIMIT", DEFAULT_TERMINATION_TIME_LIMIT_S)},
        .common = createCommonConfig(),
    };
}

} // namespace vm_scheduler
