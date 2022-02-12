#pragma once

#include "libs/scheduler/include/create_vm_assigner.h"

#include <string>


namespace vm_scheduler {

struct SchedulerConfig {
    std::chrono::seconds schedulingInterval;
    VmAssignerType vmAssignerType;
};

SchedulerConfig createSchedulerConfig();

} // namespace vm_scheduler
