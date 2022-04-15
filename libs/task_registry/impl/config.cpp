#include "libs/task_registry/include/config.h"

#include <libs/common/include/env.h>

namespace vm_scheduler {

namespace {

constexpr size_t DEFAULT_VMS_ALLOCATION_INTERVAL_S{60};
constexpr size_t DEFAULT_VMS_SCHEDULE_INTERVAL_S{60};
constexpr size_t DEFAULT_DETECT_FAILURES_INTERVAL_S{60};

} // anonymous namespace

Config createConfig()
{
    return {
        .allocationInterval =
            std::chrono::seconds{getFromEnvOrDefault("VMS_ALLOCATION_INTERVAL_S", DEFAULT_VMS_ALLOCATION_INTERVAL_S)},
        .schduleInterval =
            std::chrono::seconds{getFromEnvOrDefault("VMS_SCHEDULE_INTERVAL_S", DEFAULT_VMS_SCHEDULE_INTERVAL_S)},
        .detectFailuresInterval =
            std::chrono::seconds{
                getFromEnvOrDefault("VMS_DETECT_FAILURES_INTERVAL_S", DEFAULT_DETECT_FAILURES_INTERVAL_S)},
    };
}

} // namespace vm_scheduler
