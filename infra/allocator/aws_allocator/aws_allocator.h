#pragma once

#include "aws_ec2_config.h"

#include <lib/allocator/allocator.h>
#include <lib/common/result.h>  // TODO: remove

#include <aws/ec2/EC2Client.h>


namespace vm_scheduler::allocator {

struct AwsInstanceInfo {
    Aws::EC2::Model::InstanceType type;
    Aws::String id;
};

class AwsAllocator : public VmAllocator {
public:
    Result<InstanceInfo> allocate(const Slot& slot) override;
    Result<void> deallocate(const InstanceInfo& instanceInfo) override;
private:
    Result<AwsInstanceInfo> allocate(const Aws::EC2::Model::InstanceType& instanceType);
    Result<void> deallocate(const AwsInstanceInfo& instanceInfo);
    Result<AwsInstanceInfo> createInstance(const Aws::EC2::Model::InstanceType& instanceType);
    Result<void> startInstance(const AwsInstanceInfo& instanceInfo);
    Aws::EC2::Model::InstanceType getInstanceTypeBySlot(const Slot& slot);
private:
    Aws::EC2::EC2Client client_;
    AwsEc2Config config_;
};

} // namespace vm_scheduler::allocator
