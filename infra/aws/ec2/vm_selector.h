#pragma once

#include "config.h"

#include <lib/vm_selector/vm_storage.h>

namespace vm_scheduler::vm_selector {

class AwsVmSelector : public VmSelector {
public:
    TaskToVm select(
        const std::vector<IdleSlot>& idleSlots,
        const std::vector<task::TaskSlot>& requiredCapacities) override;
private:
    Aws::EC2::EC2Client client_;
    const allocator::AwsEc2Config& config_;
    std::vector<Aws::EC2::Model::InstanceType> availableVmTypes;
    // possible slots
};

} // namespace vm_scheduler::vm_selector
