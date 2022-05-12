#include "libs/zookeeper/include/zk_config.h"

#include <libs/common/include/env.h>

namespace vm_scheduler {

constexpr auto DEFAULT_ZK_LOCK_NAME = "";
constexpr auto DEFAULT_ZK_LOCK_PATH = "vms-prod";

ZkConfig createZkConfig()
{
    return ZkConfig{
        .lockName = getFromEnvOrDefault(
            "VMS_ZK_LOCK_NAME", DEFAULT_ZK_LOCK_NAME),
        .lockPath = getFromEnvOrDefault(
            "VMS_ZK_LOCK_PATH", DEFAULT_ZK_LOCK_PATH),
    };
}

} // namespace vm_scheduler
