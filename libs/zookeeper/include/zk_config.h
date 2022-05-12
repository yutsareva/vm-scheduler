#pragma once

#include <vector>
#include <string>

namespace vm_scheduler {

struct ZkConfig {
    std::string lockPath;
    std::vector<char> lockName = {};
};

ZkConfig createZkConfig();

} // namespace vm_scheduler
