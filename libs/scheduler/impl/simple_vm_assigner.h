#pragma once

#include "libs/scheduler/include/vm_assigner.h"


namespace vm_scheduler {

class SimpleVmAssigner : public VmAssigner {
public:
    using VmAssigner::VmAssigner;
    StateChange assign() noexcept override;
};

} // namespace vm_scheduler
