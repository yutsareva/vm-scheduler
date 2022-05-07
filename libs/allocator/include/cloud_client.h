#pragma once

#include <libs/common/include/result.h>
#include <libs/common/include/slot.h>
#include <libs/state/include/vm.h>

#include <string>

namespace vm_scheduler {

class CloudClient {
public:
    virtual ~CloudClient() = default;

    virtual Result<AllocatedVmInfo> allocate(
        const VmId vmId, const SlotCapacity& slot) noexcept = 0;
    virtual Result<void> terminate(const CloudVmId& vmId) noexcept = 0;
    virtual Result<AllocatedVmInfos> getAllAllocatedVms() noexcept = 0;
    virtual std::vector<SlotCapacity> getPossibleSlots() noexcept = 0;
};

} // namespace vm_scheduler
