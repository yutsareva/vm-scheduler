#include "libs/zookeeper/include/zk_config.h"

#include <libs/common/include/env.h>

namespace vm_scheduler {

constexpr auto DEFAULT_ZK_LOCK_NAME = "";
constexpr auto DEFAULT_ZK_LOCK_PATH = "vms-prod";
constexpr auto DEFAULT_ZK_ADDRESS = "zookeeper";

ZkConfig createZkConfig()
{
    const auto lockNameStr =
        getFromEnvOrDefault("VMS_ZK_LOCK_NAME", DEFAULT_ZK_LOCK_NAME);
    return ZkConfig{
        .address = getFromEnvOrDefault("VMS_ZK_ADDRESS", DEFAULT_ZK_ADDRESS),
        .lockPath = getFromEnvOrDefault("VMS_ZK_LOCK_PATH", DEFAULT_ZK_LOCK_PATH),
        .lockName = {lockNameStr.begin(), lockNameStr.end()},
    };
}

} // namespace vm_scheduler
