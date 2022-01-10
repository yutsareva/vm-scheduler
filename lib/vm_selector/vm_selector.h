#pragma once

#include <lib/task/task.h>

#include <vector>
#include <unordered_map>

namespace vm_scheduler::vm_selector {

using TaskToVm = std::unordered_map<task::TaskId, Vm>;

class VmSelector {
public:
    virtual TaskToVm select(
        const std::vector<IdleSlot>& idleSlots,
        const std::vector<task::TaskSlot>& requiredCapacities) = 0;
};

} // namespace vm_scheduler::vm_selector
