#pragma once

#include <string>

namespace vm_scheduler {

std::string getFromEnvOrDefault(const std::string& envVarName, const std::string& defaultValue);
std::string getFromEnvOrThrow(const std::string& envVarName);

} // namespace vm_scheduler
