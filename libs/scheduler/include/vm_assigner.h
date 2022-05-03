#pragma once

#include <libs/state/include/state.h>

#include <memory>
#include <string>


namespace vm_scheduler {

class VmAssigner {
public:
    explicit VmAssigner(State state) noexcept : state_(std::move(state)) {};
    virtual StateChange assign() noexcept = 0;
    virtual ~VmAssigner() {};

protected:
    const State state_;
};

} // namespace vm_scheduler
