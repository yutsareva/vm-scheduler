#pragma once

#include "libs/state/include/task.h"
#include "libs/state/include/vm.h"

#include <libs/common/include/slot.h>
#include <libs/common/include/types.h>

#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace vm_scheduler {

struct DesiredSlot {
    SlotCapacity total;
    SlotCapacity idle;

    bool operator==(const DesiredSlot&) const = default;
};

std::ostream& operator<<(std::ostream& out, const DesiredSlot& desiredSlot);

using VmInfo = std::variant<VmId, DesiredSlot>;
using JobToVm = std::unordered_map<JobId, VmInfo>;

struct VmCapacityUpdate {
    VmId id;
    SlotCapacity idleCapacity;

    bool operator==(const VmCapacityUpdate&) const = default;
};

std::ostream& operator<<(std::ostream& out, const VmCapacityUpdate& vmCapacityUpdate);

struct StateChange {
    JobToVm vmAssignments;
    std::vector<VmId> vmsToTerminate;
    std::vector<VmCapacityUpdate> vmCapacityUpdates;

    bool operator==(const StateChange&) const = default;
};

std::ostream& operator<<(std::ostream& out, const StateChange& state);

struct State {
    std::vector<QueuedJobInfo> queuedJobs;
    std::vector<ActiveVm> vms;

    bool operator==(const State&) const = default;
};

std::ostream& operator<<(std::ostream& out, const State& state);

} // namespace vm_scheduler
