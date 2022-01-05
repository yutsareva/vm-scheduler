#include "aws_allocator.h"

#include <lib/common/stringify.h>

#include <aws/ec2/model/RunInstancesRequest.h>
#include <aws/ec2/model/StartInstancesRequest.h>
#include <aws/ec2/model/StopInstancesRequest.h>

namespace vm_scheduler::allocator {

namespace {

InstanceInfo convertAwsVmInfoToVmInfo(const AwsInstanceInfo awsVmInfo)
{
    return {
        .type = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(awsVmInfo.type),
        .id = awsVmInfo.id,
    };
}

AwsInstanceInfo convertVmInfoToAwsVmInfo(const InstanceInfo vmInfo)
{
    return {
        .type = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName(vmInfo.type),
        .id = vmInfo.id,
    };
}

} // anonymous namespace

Result<AwsInstanceInfo> AwsAllocator::allocate(const Aws::EC2::Model::InstanceType& instanceType)
{
    const auto instanceInfo = createInstance(instanceType);
    if (instanceInfo.IsFailure()) {
        return instanceInfo;
    }
    const auto startInstanceResult = startInstance(instanceInfo.ValueRefOrThrow());
    if (startInstanceResult.IsFailure()) {
        // TODO: deallocate
    }
    return instanceInfo;
}

Result<void> AwsAllocator::deallocate(const AwsInstanceInfo& awsVmInfo)
{
    Aws::EC2::Model::StopInstancesRequest request;
    request.AddInstanceIds(awsVmInfo.id);

    const auto result = client_.StopInstances(request);
    if (result.IsSuccess()) {
        return Result<void>::Success();
    }
    const auto instanceTypeName =  Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(awsVmInfo.type);
    return Result<void>::Failure(toString(
        "Failed to stop ec2 instance ", instanceTypeName,
        " based on ami ", config_.amiId, ":",
        result.GetError().GetMessage()));
}

Result<InstanceInfo> AwsAllocator::allocate(const vm_scheduler::allocator::Slot& slot) {
    const auto instanceType = getInstanceTypeBySlot(slot);
    auto result = allocate(instanceType);
    if (result.IsSuccess()) {
        return convertAwsVmInfoToVmInfo(std::move(result).ValueOrThrow());
    }
    return Result<InstanceInfo>::Failure(std::move(result).ErrorOrThrow());
}

Result<void> AwsAllocator::deallocate(const InstanceInfo& instanceInfo)
{
    const auto awsInstanceInfo = convertVmInfoToAwsVmInfo(instanceInfo);
    return deallocate(awsInstanceInfo);
}

Aws::EC2::Model::InstanceType AwsAllocator::getInstanceTypeBySlot(const Slot&)
{
    // TODO: implement
    return Aws::EC2::Model::InstanceType::t1_micro;
}

Result<AwsInstanceInfo> AwsAllocator::createInstance(const Aws::EC2::Model::InstanceType& instanceType)
{
    const auto instanceTypeName = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(instanceType);

    Aws::EC2::Model::RunInstancesRequest runRequest;
    runRequest.WithImageId(config_.amiId)
        .WithInstanceType(instanceType)
        .WithMinCount(1)
        .WithMaxCount(1);

    auto result = client_.RunInstances(runRequest);
    if (!result.IsSuccess()) {
        return Result<AwsInstanceInfo>::Failure(toString(
            "Failed to start ec2 instance ", instanceTypeName,
            " based on ami ", config_.amiId, ":",
            result.GetError().GetMessage()));
    }

    const auto& instances = result.GetResult().GetInstances();
    if (instances.size() == 0) {
        return Result<AwsInstanceInfo>::Failure(toString(
            "Failed to start ec2 instance ", instanceTypeName,
            " based on ami ", config_.amiId, ":",
            result.GetError().GetMessage()));
    }

    const auto instanceId = instances[0].GetInstanceId();
    std::cout << "Successfully started ec2 instance " << instanceId
              << " with type: " << instanceTypeName
              << " based on ami " << config_.amiId << std::endl;
    return Result{AwsInstanceInfo{
        .type = instanceType,
        .id = instanceId
    }};
}

Result<void> AwsAllocator::startInstance(const AwsInstanceInfo& instanceInfo)
{
    Aws::EC2::Model::StartInstancesRequest startRequest;
    startRequest.AddInstanceIds(instanceInfo.id);

    auto result = client_.StartInstances(startRequest);
    if (result.IsSuccess()) {
        return Result<void>::Success();
    }
    return Result<void>::Failure(toString(
        "Failed to start instance: ", instanceInfo.id,
        ": ", result.GetError().GetMessage()));
}

} // namespace vm_scheduler::allocator
