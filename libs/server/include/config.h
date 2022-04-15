#pragma once

#include <string>

namespace vm_scheduler {

struct ServerConfig {
    std::string address;
};

ServerConfig createServerConfig();

} // namespace vm_scheduler
