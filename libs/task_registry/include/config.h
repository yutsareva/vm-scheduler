#pragma once

#include <string>


namespace vm_scheduler {

struct Config {
    std::chrono::seconds allocationInterval;
    std::chrono::seconds schduleInterval;
};

Config createConfig();

} // namespace vm_scheduler
