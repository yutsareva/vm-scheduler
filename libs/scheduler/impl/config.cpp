#include "libs/scheduler/include/config.h"
#include "libs/scheduler/include/vm_assigner.h"

#include <libs/common/include/env.h>


namespace vm_scheduler {

constexpr std::chrono::minutes DEFAULT_SCHEDULING_INTERVAL { 1 };

SchedulerConfig createSchedulerConfig() {
    return {
        .schedulingInterval = std::chrono::minutes{getFromEnvOrDefault(
            "VMS_SCHEDULING_INTERVAL", DEFAULT_SCHEDULING_INTERVAL.count())},
        .vmAssignerType = getFromEnvOrDefault("VMS_ASSIGNER_TYPE", VmAssignerType::Simple),
    };
}

} // namespace vm_scheduler
