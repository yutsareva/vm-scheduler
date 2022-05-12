#include "libs/scheduler/include/config.h"
#include "libs/scheduler/include/vm_assigner.h"

#include <libs/common/include/env.h>

namespace vm_scheduler {

// should be less than task_registry.config.scheduleInterval
constexpr size_t DEFAULT_SCHEDULING_INTERVAL_S { 40 };

SchedulerConfig createSchedulerConfig()
{
    const auto vmAssignerType = vmAssignerTypeFromString(getFromEnvOrDefault(
        "VMS_ASSIGNER_TYPE", toString(VmAssignerType::Complex)));
    return {
        .schedulingInterval = std::chrono::seconds{getFromEnvOrDefault(
            "VMS_SCHEDULING_INTERVAL_S", DEFAULT_SCHEDULING_INTERVAL_S)},
        .vmAssignerType = vmAssignerType,
        .complexVmAssignerConfig
            = (vmAssignerType == VmAssignerType::Complex)
            ? std::optional<ComplexVmAssignerConfig>(createComplexVmAssignerConfig())
            : std::nullopt,
    };
}

} // namespace vm_scheduler
