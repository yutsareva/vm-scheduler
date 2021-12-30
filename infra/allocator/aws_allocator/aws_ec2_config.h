#pragma once

namespace vm_scheduler::allocator {

class AwsEc2Config {
    Aws::String amiId;
    std::vector<Aws::EC2::Model::InstanceType> instanceTypes;
};

} // namespace vm_scheduler::allocator