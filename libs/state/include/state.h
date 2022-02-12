#pragma once

#include "libs/state/include/task.h"
#include "libs/state/include/vm.h"

#include <libs/common/include/slot.h>
#include <libs/common/include/types.h>

#include <unordered_map>
#include <vector>


namespace vm_scheduler {

struct DesiredSlot {
    SlotCapacity total;
    SlotCapacity idle;

    bool operator==(const DesiredSlot&) const = default;
};

using VmInfo = std::variant<VmId, DesiredSlot>;
using TaskToVm = std::unordered_map<TaskId, VmInfo>;

struct VmCapacityUpdate {
    VmId id;
    SlotCapacity idleCapacity;

    bool operator==(const VmCapacityUpdate&) const = default;
};

struct StateChange {
    TaskToVm vmAssignments;
    std::vector<VmId> vmsToTerminate;
    std::vector<VmCapacityUpdate> vmCapacityUpdates;

    bool operator==(const StateChange&) const = default;
};

struct State {
    std::vector<QueuedTaskInfo> queuedTasks;
    std::vector<ActiveVm> vms;

    bool operator==(const State&) const = default;
};

} // namespace vm_scheduler
