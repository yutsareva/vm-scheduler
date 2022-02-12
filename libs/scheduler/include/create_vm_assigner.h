#pragma once

#include "libs/scheduler/include/vm_assigner.h"

#include <libs/state/include/state.h>

#include <memory>


namespace vm_scheduler {

enum class VmAssignerType {
    Simple /* simple */,
};

std::unique_ptr<VmAssigner> createVmAssigner(const VmAssignerType type, const State& state);

} // namespace vm_scheduler
