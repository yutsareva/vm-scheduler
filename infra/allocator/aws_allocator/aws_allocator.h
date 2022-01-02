#pragma once

#include "aws_ec2_config.h"

#include <lib/allocator/allocator.h>
#include <lib/common/result.h>

#include <aws/ec2/EC2Client.h>

namespace vm_scheduler::allocator {

struct InstanceInfo {
    Aws::EC2::Model::InstanceType type;
    Aws::String id;
};

class AwsAllocator : public VmAllocator {
public:
    void allocate(const Slot& slot) override;
    void deallocate(const Slot& slot) override;
private:
    Result<InstanceInfo> createInstance(const Slot& slot);
    Aws::EC2::Model::InstanceType getInstanceTypeBySlot(const Slot& slot);
private:
    Aws::EC2::EC2Client client_;
    AwsEc2Config config_;
};

} // namespace vm_scheduler::allocator
