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

struct DesiredSlotId {
    explicit constexpr DesiredSlotId(size_t id)
        : value(id) {};
    bool operator==(const DesiredSlotId&) const = default;

    size_t value;
};

} // namespace vm_scheduler

template<>
struct std::hash<vm_scheduler::DesiredSlotId> {
    size_t operator()(const vm_scheduler::DesiredSlotId& id) const
    {
        return std::hash<size_t>()(id.value);
    }
};

namespace vm_scheduler {

using DesiredSlotMap = std::unordered_map<DesiredSlotId, DesiredSlot>;
using VmInfo = std::variant<VmId, DesiredSlotId>;
using JobToVm = std::unordered_map<JobId, VmInfo>;
using VmIdToCapacity = std::unordered_map<VmId, SlotCapacity>;

//struct JobVmAssignments {
//JobToVm jobToVm;
//DesiredSlotMap desiredSlotMap;
//VmIdToCapacity allocatedVmIdToUpdatedIdleCapacity;
//};

//struct VmCapacityUpdate {
//    VmId id;
//    SlotCapacity idleCapacity;
//
//    bool operator==(const VmCapacityUpdate&) const = default;
//};

//std::ostream& operator<<(std::ostream& out, const VmCapacityUpdate& vmCapacityUpdate);

struct StateChange {
    JobToVm jobToVm;
    DesiredSlotMap desiredSlotMap;
    VmIdToCapacity allocatedVmIdToUpdatedIdleCapacity;
    std::vector<VmId> vmsToTerminate;

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
