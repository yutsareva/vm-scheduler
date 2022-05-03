#pragma once

#include "libs/scheduler/include/vm_assigner.h"

#include <libs/common/include/stringify.h>
#include <libs/state/include/state.h>

#include <string>
#include <memory>


namespace vm_scheduler {

enum class VmAssignerType {
    Simple /* simple */,
};

const char* toString(const VmAssignerType v);
VmAssignerType vmAssignerTypeFromString(const std::string& str);

std::unique_ptr<VmAssigner> createVmAssigner(const VmAssignerType type, State state);

} // namespace vm_scheduler
