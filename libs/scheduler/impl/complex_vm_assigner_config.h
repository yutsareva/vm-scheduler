#pragma once

#include <string>

namespace vm_scheduler {

enum class JobOrdering {
    Fifo /*"fifo"*/,
    MinMin /*"minmin"*/,
    MaxMin /*"maxmin"*/,
};

JobOrdering jobOrderingFromString(const std::string& s);

enum class AllocationStrategy {
    FirstFit /*"firstfit"*/,
    NextFit /*"nextfit"*/,
    WorstFit /*"worstfit"*/,
    BestFit /*"bestfit"*/,
};

struct ComplexVmAssignerConfig {
    JobOrdering jobOrdering;
    AllocationStrategy allocationStrategy;
};

ComplexVmAssignerConfig createComplexVmAssignerConfig();

} // namespace vm_scheduler
