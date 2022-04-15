#include "libs/scheduler/include/config.h"
#include "libs/scheduler/include/vm_assigner.h"

#include <libs/common/include/env.h>


namespace vm_scheduler {

constexpr size_t DEFAULT_SCHEDULING_INTERVAL_S { 60 };

SchedulerConfig createSchedulerConfig() {
    return {
        .schedulingInterval = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_SCHEDULING_INTERVAL_S", DEFAULT_SCHEDULING_INTERVAL_S)},
        .vmAssignerType = vmAssignerTypeFromString(getFromEnvOrDefault("VMS_ASSIGNER_TYPE", toString(VmAssignerType::Simple))),
    };
}

} // namespace vm_scheduler
