#include "libs/scheduler/include/create_vm_assigner.h"
#include "libs/scheduler/impl/simple_vm_assigner.h"


namespace vm_scheduler {

std::unique_ptr<VmAssigner> createVmAssigner(const VmAssignerType type, const State& state)
{
    switch (type) {
        case VmAssignerType::Simple:
            return std::make_unique<SimpleVmAssigner>(state);
    }
}

} // namespace vm_scheduler
