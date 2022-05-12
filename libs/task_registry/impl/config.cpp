#include "libs/task_registry/include/config.h"

#include <libs/common/include/env.h>

namespace vm_scheduler {

namespace {

constexpr size_t DEFAULT_VMS_ALLOCATION_INTERVAL_S{60};
constexpr size_t DEFAULT_VMS_SCHEDULE_INTERVAL_S{60};
constexpr size_t DEFAULT_DETECT_FAILURES_INTERVAL_S{300};
constexpr const char* DEFAULT_SCHEDULER_MODE = "full";
constexpr size_t DEFAULT_USE_ZK_DIST_LOCK = 0;

} // anonymous namespace

SchedulerMode schedulerModeFromString(const std::string& value)
{
    const static std::unordered_map<std::string, SchedulerMode> map = {
        {"service", SchedulerMode::SchedulerService},
        {"core", SchedulerMode::CoreScheduler},
        {"full", SchedulerMode::FullScheduler},
    };
    return map.at(value);
}

Config createConfig()
{
    return {
        .allocationInterval = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_ALLOCATION_INTERVAL_S", DEFAULT_VMS_ALLOCATION_INTERVAL_S)},
        .scheduleInterval = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_SCHEDULE_INTERVAL_S", DEFAULT_VMS_SCHEDULE_INTERVAL_S)},
        .detectFailuresInterval = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_DETECT_FAILURES_INTERVAL_S", DEFAULT_DETECT_FAILURES_INTERVAL_S)},
        .mode = schedulerModeFromString(
            getFromEnvOrDefault("VMS_MODE", DEFAULT_SCHEDULER_MODE)),
        .useZkDistLock = bool{getFromEnvOrDefault<size_t>(
            "VMS_USE_ZK_DIST_LOCK", DEFAULT_USE_ZK_DIST_LOCK)},
    };
}

} // namespace vm_scheduler
