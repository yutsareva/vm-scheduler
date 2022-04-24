#pragma once

#include <libs/state/include/vm.h>

#include <aws/ec2/model/InstanceType.h>

namespace vm_scheduler {

struct AwsVmInfo {
    Aws::EC2::Model::InstanceType type;
    Aws::String id;
};

AllocatedVmInfo convertAwsVmInfoToVmInfo(const AwsVmInfo& awsVmInfo);

} // namespace vm_scheduler
