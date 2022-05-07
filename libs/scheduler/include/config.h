#pragma once

#include "libs/scheduler/include/create_vm_assigner.h"
#include "libs/scheduler/impl/complex_vm_assigner_config.h"

#include <string>


namespace vm_scheduler {

struct SchedulerConfig {
    std::chrono::seconds schedulingInterval;
    VmAssignerType vmAssignerType;
    std::optional<ComplexVmAssignerConfig> complexVmAssignerConfig;
};

SchedulerConfig createSchedulerConfig();

} // namespace vm_scheduler
