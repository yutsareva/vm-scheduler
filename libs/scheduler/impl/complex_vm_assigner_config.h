#pragma once

#include <string>

namespace vm_scheduler {

enum class JobOrdering {
    Fifo /*"fifo"*/,
    Ascending /*"minmin"*/,
    Descending /*"maxmin"*/,
};

JobOrdering jobOrderingFromString(const std::string& s);

enum class AllocationStrategy {
    FirstFit /*"firstfit"*/,
    NextFit /*"nextfit"*/,
    WorstFit /*"worstfit"*/,
    BestFit /*"bestfit"*/,
};

AllocationStrategy allocationStrategyFromString(const std::string& s);

struct ComplexVmAssignerConfig {
    JobOrdering jobOrdering;
    AllocationStrategy allocationStrategy;
};

ComplexVmAssignerConfig createComplexVmAssignerConfig();

} // namespace vm_scheduler
