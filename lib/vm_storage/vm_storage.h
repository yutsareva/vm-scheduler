#pragma once

#include <vector>

namespace vm_scheduler::vm_storage {

using VmId = std::string;

enum class Cloud { Aws, Azure };

struct VmInfo {
    VmId id;
    Cloud cloud;
    std::chrono::time_point<std::chrono::steady_clock> allocatedAt;
    CpuCores cpuCores;
    GigaBytes memory;
    CpuCores idleCpuCores;
    GigaBytes idleMemory;
    // status
};

struct IdleSlot {
    VmId id;
    Cloud cloud;
    CpuCores cpuCores;
    GigaBytes memory;
};

class VmStorage {
public:
    virtual void save(const VmInfo& vmInfo) = 0;
    virtual std::vector<IdleSlot> getSortedIdleSlots(const size_t limit) = 0;
};

} // namespace vm_scheduler::vm_storage