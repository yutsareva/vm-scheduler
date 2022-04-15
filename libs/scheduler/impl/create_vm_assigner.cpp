#include "libs/scheduler/include/create_vm_assigner.h"
#include "libs/scheduler/impl/simple_vm_assigner.h"

#include <unordered_map>

namespace vm_scheduler {

const char* toString(const VmAssignerType v)
{
    switch(v) {
        case VmAssignerType::Simple: return "simple";
    }
}

VmAssignerType vmAssignerTypeFromString(const std::string& str)
{
    static const std::unordered_map<std::string, VmAssignerType> map = {
        {"simple", VmAssignerType::Simple},
    };
    return map.at(str);
}

std::unique_ptr<VmAssigner> createVmAssigner(const VmAssignerType type, const State& state)
{
    switch (type) {
        case VmAssignerType::Simple:
            return std::make_unique<SimpleVmAssigner>(state);
    }
}

} // namespace vm_scheduler
