#pragma once

#include <chrono>
#include <string>

namespace vm_scheduler {

struct Config {
    std::chrono::seconds allocationInterval;
    std::chrono::seconds schduleInterval;
    std::chrono::seconds detectFailuresInterval;
};

Config createConfig();

} // namespace vm_scheduler
