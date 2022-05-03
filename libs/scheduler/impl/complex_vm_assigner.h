#pragma once

#include "libs/scheduler/include/vm_assigner.h"


namespace vm_scheduler {

enum class JobOrdering {
    Fifo /*"fifo"*/,
    MinMin /*"minmin"*/,
    MaxMin /*"maxmin"*/,
};

JobOrdering jobOrderingFromString(const std::string& s) {
    const static std::unordered_map<std::string, OrdererType> map = {
        {"fifo", OrdererType::Fifo},
        {"minmin", OrdererType::MinMin},
        {"maxmin", OrdererType::MaxMin},
    };
    return map.at(s);
}

struct ComplexVmAssignerConfig {
    JobOrdering jobOrdering;
    AllocationStrategy allocationStrategy;
};

ComplexVmAssignerConfig createComplexVmAssignerConfig()
{
    return ComplexVmAssignerConfig{
        .jobOrdering = jobOrderingFromString(getFromEnvOrDefault(
            "VMS_JOB_ORDERING", "fifo")),

    };
}

class ComplexVmAssigner : public VmAssigner {
public:
    ComplexVmAssigner(const ComplexVmAssignerConfig& config, State state);
    StateChange assign() noexcept override;
private:
    OrderedJobs orderedJobs_;
    JobAllocator jobAllocator_;
};

} // namespace vm_scheduler
