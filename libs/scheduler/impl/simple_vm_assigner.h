#pragma once

#include "libs/scheduler/include/vm_assigner.h"

namespace vm_scheduler {

class SimpleVmAssigner : public VmAssigner {
public:
    explicit SimpleVmAssigner(State state) : state_(std::move(state)){};
    StateChange assign() noexcept override;

private:
    const State state_;
};

} // namespace vm_scheduler
