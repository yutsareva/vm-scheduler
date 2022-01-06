#pragma once

#include <aws/ec2/EC2Client.h>

namespace vm_scheduler::allocator {

Aws::EC2::EC2Client createEc2Client();

} // namespace vm_scheduler::allocator
