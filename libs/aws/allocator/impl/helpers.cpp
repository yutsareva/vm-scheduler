#include "libs/aws/allocator/impl/helpers.h"

namespace vm_scheduler {

AllocatedVmInfo convertAwsVmInfoToVmInfo(const AwsVmInfo& awsVmInfo)
{
    return {
        .id = awsVmInfo.id,
        .type = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(
            awsVmInfo.type),
    };
}

} // namespace vm_scheduler
