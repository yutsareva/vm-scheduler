#include "aws_allocator.h"

#include <lib/common/stringify.h>

#include <aws/ec2/model/RunInstancesRequest.h>

namespace vm_scheduler::allocator {

void AwsAllocator::allocate(const vm_scheduler::allocator::Slot& slot) { }

void AwsAllocator::deallocate(const Slot& slot) { }

Aws::EC2::Model::InstanceType AwsAllocator::getInstanceTypeBySlot(const Slot& slot)
{
    // TODO: implement
    return Aws::EC2::Model::InstanceType::t1_micro;
}

Result<InstanceInfo> AwsAllocator::createInstance(const Slot& slot)
{
    const auto instanceType = getInstanceTypeBySlot(slot);
    const auto instanceTypeName = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(instanceType);

    Aws::EC2::Model::RunInstancesRequest runRequest;
    runRequest.WithImageId(config_.amiId)
        .WithInstanceType(instanceType)
        .WithMinCount(1)
        .WithMaxCount(1);

    auto result = client_.RunInstances(runRequest);
    if (!result.IsSuccess()) {
        return Result<InstanceInfo>::Failure(toString(
            "Failed to start ec2 instance ", instanceTypeName,
            " based on ami ", config_.amiId, ":",
            result.GetError().GetMessage()
        ));
    }

    const auto& instances = result.GetResult().GetInstances();
    if (instances.size() == 0) {
        return Result<InstanceInfo>::Failure(toString(
            "Failed to start ec2 instance ", instanceTypeName,
            " based on ami ", config_.amiId, ":",
            result.GetError().GetMessage()
        ));
    }

    std::cout << "Successfully started ec2 instance " << instanceTypeName
              << " based on ami " << config_.amiId << std::endl;
    return Result{InstanceInfo{
        .type = instanceType,
        .id = instances[0].GetInstanceId()
    }};
}

} // namespace vm_scheduler::allocator
