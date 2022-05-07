#include "libs/scheduler/include/create_vm_assigner.h"
#include "libs/scheduler/impl/max_min_best_fit_vm_assigner.h"
#include "libs/scheduler/impl/simple_vm_assigner.h"

#include <unordered_map>

namespace vm_scheduler {

const char* toString(const VmAssignerType v)
{
    switch (v) {
        case VmAssignerType::Simple:
            return "simple";
        case VmAssignerType::Complex:
            return "complex";
    }
}

VmAssignerType vmAssignerTypeFromString(const std::string& str)
{
    static const std::unordered_map<std::string, VmAssignerType> map = {
        {"simple", VmAssignerType::Simple},
        {"complex", VmAssignerType::Complex},
    };
    return map.at(str);
}

std::unique_ptr<VmAssigner> createVmAssigner(
    const VmAssignerType type,
    State state,
    const std::optional<ComplexVmAssignerConfig>& complexVmAssignerConfig,
    const std::vector<SlotCapacity>& possibleSlots)
{
    switch (type) {
        case VmAssignerType::Simple:
            return std::make_unique<SimpleVmAssigner>(std::move(state));
        case VmAssignerType::Complex:
            return std::make_unique<ComplexVmAssigner>(
                *complexVmAssignerConfig, std::move(state), possibleSlots);
    }
}

} // namespace vm_scheduler
