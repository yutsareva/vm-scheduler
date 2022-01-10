#pragma once

#include "config.h"

#include <lib/allocator/allocator.h>

#include <aws/ec2/EC2Client.h>

namespace vm_scheduler::allocator {

struct AwsVmInfo {
    Aws::EC2::Model::InstanceType type;
    Aws::String id;
};

class AwsAllocator : public VmAllocator {
public:
    AwsAllocator();
    Result<VmInfo> allocate(const SlotCapacity& slot) override;
    Result<void> deallocate(const VmInfo& instanceInfo) override;

private:
    Result<AwsVmInfo> allocate(const Aws::EC2::Model::InstanceType& instanceType);
    Result<void> deallocate(const AwsVmInfo& instanceInfo);
    Result<AwsVmInfo> createInstance(const Aws::EC2::Model::InstanceType& instanceType);
    Result<void> startInstance(const AwsVmInfo& instanceInfo);

private:
    Aws::EC2::EC2Client client_;
    AwsEc2Config config_;
};

} // namespace vm_scheduler::allocator
