#include "libs/scheduler/impl/complex_vm_assigner_config.h"
#include <libs/common/include/env.h>

#include <unordered_map>

namespace vm_scheduler {

JobOrdering jobOrderingFromString(const std::string& s)
{
    const static std::unordered_map<std::string, JobOrdering> map = {
        {"fifo", JobOrdering::Fifo},
        {"minmin", JobOrdering::MinMin},
        {"maxmin", JobOrdering::MaxMin},
    };
    return map.at(s);
}

ComplexVmAssignerConfig createComplexVmAssignerConfig()
{
    return ComplexVmAssignerConfig{
        .jobOrdering = jobOrderingFromString(
            getFromEnvOrDefault("VMS_JOB_ORDERING", "fifo")),

    };
}

} // namespace vm_scheduler
