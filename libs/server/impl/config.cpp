#include "libs/server/include/config.h"

#include <libs/common/include/env.h>

namespace vm_scheduler {
namespace {

const std::string DEFAULT_GRPC_SERVER_ADDRESS = "0.0.0.0:50001";

} // anonymous namespace

ServerConfig createServerConfig()
{
    return {
        .address = getFromEnvOrDefault(
            "VMS_GRPC_SERVER_ADDRESS", DEFAULT_GRPC_SERVER_ADDRESS),
    };
}

} // namespace vm_scheduler
