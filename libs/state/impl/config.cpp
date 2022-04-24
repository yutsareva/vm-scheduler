#include <libs/state/include/config.h>

#include <libs/common/include/env.h>

namespace vm_scheduler {

namespace {

constexpr size_t DEFAULT_JOB_RESTART_ATTEMPT_COUNT{3};
constexpr size_t DEFAULT_VM_RESTART_ATTEMPT_COUNT{3};

} // anonymous namespace

CommonConfig createCommonConfig()
{
    return {
        .jobRestartAttemptCount = getFromEnvOrDefault(
            "VMS_JOB_RESTART_ATTEMPT_COUNT", DEFAULT_JOB_RESTART_ATTEMPT_COUNT),
        .vmRestartAttemptCount = getFromEnvOrDefault(
            "VMS_VM_RESTART_ATTEMPT_COUNT", DEFAULT_VM_RESTART_ATTEMPT_COUNT),
    };
}

} // namespace vm_scheduler
