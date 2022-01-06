#include "env.h"
#include "stringify.h"

#include <cstdlib>

namespace vm_scheduler {

std::string getFromEnvOrDefault(const std::string& envVarName, const std::string& defaultValue)
{
    auto envVarValue = std::getenv(envVarName.c_str());
    if (envVarValue) {
        return envVarValue;
    }
    return defaultValue;
}

std::string getFromEnvOrThrow(const std::string& envVarName)
{
    auto envVarValue = std::getenv(envVarName.c_str());
    if (envVarValue) {
        return envVarValue;
    }
    throw std::runtime_error(toString("Env var `", envVarName, "` must be specified."));
}

} // namespace vm_scheduler