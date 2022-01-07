#include "allocator.h"
#include "client.h"
#include "config.h"

#include <lib/common/stringify.h>
#include <lib/common/errors.h>

#include <aws/ec2/model/RunInstancesRequest.h>
#include <aws/ec2/model/StartInstancesRequest.h>
#include <aws/ec2/model/TerminateInstancesRequest.h>

namespace vm_scheduler::allocator {

namespace {

InstanceInfo convertAwsVmInfoToVmInfo(const AwsInstanceInfo& awsVmInfo)
{
    return {
        .type = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(awsVmInfo.type),
        .id = awsVmInfo.id,
    };
}

AwsInstanceInfo convertVmInfoToAwsVmInfo(const InstanceInfo& vmInfo)
{
    return {
        .type = Aws::EC2::Model::InstanceTypeMapper::GetInstanceTypeForName(vmInfo.type),
        .id = vmInfo.id,
    };
}

Aws::EC2::Model::InstanceType getInstanceTypeBySlot(const Slot&)
{
    // TODO: implement
    return Aws::EC2::Model::InstanceType::t1_micro;
}

} // anonymous namespace

AwsAllocator::AwsAllocator()
    : client_(createEc2Client())
    , config_(createEc2Config())
{
}

Result<AwsInstanceInfo> AwsAllocator::allocate(const Aws::EC2::Model::InstanceType& instanceType)
{
    auto instanceInfo = createInstance(instanceType);
    if (instanceInfo.IsFailure()) {
        return instanceInfo;
    }
    const auto startInstanceResult = startInstance(instanceInfo.ValueRefOrThrow());
    if (startInstanceResult.IsFailure()) {
        std::cerr << "Failed to start instance `" << instanceInfo.ValueRefOrThrow().id << "`, deallocating..." << std::endl;
        auto deallocateResult = deallocate(instanceInfo.ValueRefOrThrow());
        if (deallocateResult.IsFailure()) {
            return Result<AwsInstanceInfo>::Failure<AllocationException>(
                "Allocation failed: instance start failed, instance deallocation failed",
                std::move(deallocateResult).ErrorOrThrow());
        }
        return Result<AwsInstanceInfo>::Failure<AllocationException>(
            "Allocation failed: instance start failed, while instance successfully deallocated",
            std::move(deallocateResult).ErrorOrThrow());
    }
    return instanceInfo;
}

Result<void> AwsAllocator::deallocate(const AwsInstanceInfo& awsVmInfo)
{
    Aws::EC2::Model::TerminateInstancesRequest request;
    request.AddInstanceIds(awsVmInfo.id);

    const auto result = client_.TerminateInstances(request);
    if (result.IsSuccess()) {
        return Result<void>::Success();
    }
    const auto instanceTypeName =  Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(awsVmInfo.type);
    return Result<void>::Failure<AllocationException>(toString(
        "Failed to terminate ec2 instance ", instanceTypeName,
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
        return Result<AwsInstanceInfo>::Failure<AllocationException>(toString(
            "Failed to start ec2 instance ", instanceTypeName,
            " based on ami ", config_.amiId, ":",
            result.GetError().GetMessage()));
    }

    const auto& instances = result.GetResult().GetInstances();
    if (instances.empty()) {
        return Result<AwsInstanceInfo>::Failure<AllocationException>(toString(
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
    return Result<void>::Failure<AllocationException>(toString(
        "Failed to start instance: ", instanceInfo.id,
        ": ", result.GetError().GetMessage()));
}

} // namespace vm_scheduler::aws
