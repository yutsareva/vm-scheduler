#pragma once

#include <libs/common/include/slot.h>
#include <libs/common/include/types.h>

#include <unordered_set>
#include <vector>

namespace vm_scheduler {

using VmId = int64_t;
using CloudVmId = std::string;
using VmType = std::string;

struct AllocatedVmInfo {
    CloudVmId id;
    VmType type;

    bool operator==(const AllocatedVmInfo&) const = default;
};

using AllocatedVmInfos = std::unordered_set<AllocatedVmInfo>;

} // namespace vm_scheduler

template<>
struct std::hash<vm_scheduler::AllocatedVmInfo> {
    size_t operator()(const vm_scheduler::AllocatedVmInfo& a) const
    {
        return std::hash<std::string>()(a.id);
    }
};

namespace vm_scheduler {

std::ostream& operator<<(std::ostream& out, const AllocatedVmInfos& vms);

struct AllocationPendingVmInfo {
    VmId id;
    SlotCapacity capacity;

    bool operator==(const AllocationPendingVmInfo&) const = default;
};

struct TerminationPendingVmInfo {
    VmId id;
    CloudVmId cloudVmId;

    bool operator==(const TerminationPendingVmInfo&) const = default;
};

enum class VmStatus {
    PendingAllocation /* "pending_allocation" */,
    Allocating /* "allocating" */,
    Allocated /* "allocated" */,
    AgentStarted /* "agent_started" */,
    PendingTermination /* "pending_termination" */,
    Terminating /* "terminating" */,
    Terminated /* "terminated" */,
};

const char* toString(VmStatus v);

VmStatus vmStatusFromString(const std::string& s);

template<typename Status>
Status fromString(const std::string& s);

template<>
VmStatus fromString(const std::string& s);

struct ActiveVm { // status: pending_allocation or allocated
    VmId id;
    SlotCapacity totalCapacity;
    SlotCapacity idleCapacity;

    bool operator==(const ActiveVm&) const = default;
};

std::ostream& operator<<(std::ostream& out, const std::vector<ActiveVm>& activeVms);

const std::unordered_set<VmStatus>&  getActiveVmStatuses();

} // namespace vm_scheduler
