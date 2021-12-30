#pragma once

#include "aws_ec2_config.h"

#include "lib/allocator/allocator.h"

#include <aws/ec2/EC2Client.h>

namespace vm_scheduler::allocator {

class AwsAllocator : public VmAllocator {
public:
    void allocate(const Slot& slot) override;
    void deallocate(const Slot& slot) override;
private:
    void createInstance(const Slot& slot);
private:
    Aws::EC2::EC2Client client_;
    AwsEc2Config config_;
};

} // namespace vm_scheduler::allocator
