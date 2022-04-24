#pragma once

#include <chrono>
#include <string>

namespace vm_scheduler {

enum class SchedulerMode {
    SchedulerService /* "service" */,
    CoreScheduler /* "core" */,
    FullScheduler /* "full" */,
};

SchedulerMode schedulerModeFromString(const std::string& value);

struct Config {
    std::chrono::seconds allocationInterval;
    std::chrono::seconds scheduleInterval;
    std::chrono::seconds detectFailuresInterval;
    SchedulerMode mode;
};

Config createConfig();

} // namespace vm_scheduler
