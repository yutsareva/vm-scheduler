#pragma once

#include <cstdio>


namespace vm_scheduler {

struct CommonConfig {
    size_t jobRestartAttemptCount;
    size_t vmRestartAttemptCount;
};

CommonConfig createCommonConfig();

} // namespace vm_scheduler
