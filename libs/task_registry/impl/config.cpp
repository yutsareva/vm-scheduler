#include "libs/task_registry/include/config.h"

#include <libs/common/include/env.h>


namespace vm_scheduler {

constexpr size_t DEFAULT_VMS_ALLOCATION_INTERVAL { 60 };
constexpr size_t DEFAULT_VMS_SCHEDULE_INTERVAL { 60 };

Config createConfig() {
    return {
        .allocationInterval = std::chrono::seconds{
            getFromEnvOrDefault("VMS_ALLOCATION_INTERVAL", DEFAULT_VMS_ALLOCATION_INTERVAL)},
        .schduleInterval = std::chrono::seconds{
            getFromEnvOrDefault("VMS_SCHEDULE_INTERVAL", DEFAULT_VMS_SCHEDULE_INTERVAL)},
    };
}

} // namespace vm_scheduler
