#pragma once

#include "libs/scheduler/include/vm_assigner.h"

#include <libs/common/include/stringify.h>
#include <libs/state/include/state.h>

#include <libs/scheduler/impl/complex_vm_assigner_config.h>
#include <memory>
#include <string>

namespace vm_scheduler {

enum class VmAssignerType {
    Simple /* simple */,
    Complex /* complex */,
};

const char* toString(const VmAssignerType v);
VmAssignerType vmAssignerTypeFromString(const std::string& str);

std::unique_ptr<VmAssigner> createVmAssigner(
    const VmAssignerType type,
    State state,
    const std::optional<ComplexVmAssignerConfig>& complexVmAssignerConfig,
    const std::vector<SlotCapacity>& possibleSlots);

} // namespace vm_scheduler
