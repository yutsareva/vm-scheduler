#pragma once

#include "libs/common/include/stringify.h"

#include <string>
#include <type_traits>

namespace vm_scheduler {

std::string getFromEnvOrDefault(const std::string& envVarName, const std::string& defaultValue);
size_t getFromEnvOrDefault(const std::string& envVarName, const size_t defaultValue);
std::string getFromEnvOrThrow(const std::string& envVarName);

} // namespace vm_scheduler
