#pragma once

#include <libs/state/include/state.h>

#include <memory>
#include <string>


namespace vm_scheduler {

class VmAssigner {
public:
    virtual StateChange assign() noexcept = 0;
    virtual ~VmAssigner() {};

};

} // namespace vm_scheduler
