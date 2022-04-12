#include "libs/aws/allocator/include/cloud_client.h"
#include "libs/aws/allocator/impl/errors.h"

#include <libs/common/include/env.h>

#include <libs/common/include/log.h>

#include <aws/core/auth/AWSCredentials.h>
#include <aws/ec2/model/RunInstancesRequest.h>
#include <aws/ec2/model/StartInstancesRequest.h>
#include <aws/ec2/model/TerminateInstancesRequest.h>

namespace vm_scheduler {

namespace {

AllocatedVmInfo convertAwsVmInfoToVmInfo(const AwsVmInfo& awsVmInfo)
{
    return {
        .id = awsVmInfo.id,
        .type = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(awsVmInfo.type),
    };
}

Aws::EC2::Model::InstanceType getInstanceTypeBySlot(const SlotCapacity&)
{
    // TODO: implement
    return Aws::EC2::Model::InstanceType::t2_micro;
}

Aws::EC2::Model::Placement getPlacement()
{
    // TODO: in a bright future Allocater should allocate instances in different availability zones
    // for jobs from one task
    Aws::EC2::Model::Placement placement;
    placement.SetAvailabilityZone("us-east-2a");
    return placement;
}

} // anonymous namespace

AwsCloudClient::AwsCloudClient(AwsEc2Config config) : config_(std::move(config)), options_()
{
    Aws::InitAPI(options_);
    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.verifySSL = false;
    client_ = std::make_unique<Aws::EC2::EC2Client>(
        Aws::Auth::AWSCredentials{config_.accessKeyId, config_.secretKey, config_.sessionToken}, clientConfig);
}

AwsCloudClient::~AwsCloudClient()
{
    Aws::ShutdownAPI(options_);
}

Result<AllocatedVmInfo> AwsCloudClient::allocate(const SlotCapacity& slot) noexcept
{
    const auto vmType = getInstanceTypeBySlot(slot);
    auto result = allocate(vmType);
    if (result.IsSuccess()) {
        return Result{convertAwsVmInfoToVmInfo(std::move(result).ValueOrThrow())};
    }
    return Result<AllocatedVmInfo>::Failure(std::move(result).ErrorOrThrow());
}

Result<AwsVmInfo> AwsCloudClient::allocate(const Aws::EC2::Model::InstanceType& vmType)
{
    auto vmInfo = createInstance(vmType);
    if (vmInfo.IsFailure()) {
        return vmInfo;
    }

    auto startInstanceResult = startInstance(vmInfo.ValueRefOrThrow());
    if (startInstanceResult.IsFailure()) {
        ERROR() << "Failed to start vm " << vmInfo.ValueRefOrThrow().id << ", terminating...";

        auto terminateResult = terminate(vmInfo.ValueRefOrThrow().id);
        if (terminateResult.IsFailure()) {
            return Result<AwsVmInfo>::Failure<AllocationException>(toString(
                "Allocation failed: vm start failed: ",
                what(std::move(startInstanceResult).ErrorOrThrow()),
                ", then vm termination failed",
                what(std::move(terminateResult).ErrorOrThrow())));
        }

        return Result<AwsVmInfo>::Failure<AllocationException>(toString(
            "Allocation failed: vm start failed, while vm was successfully terminated: ",
            what(std::move(startInstanceResult).ErrorOrThrow())));
    }
    return vmInfo;
}

Result<AwsVmInfo> AwsCloudClient::createInstance(const Aws::EC2::Model::InstanceType& vmType)
{
    Aws::EC2::Model::RunInstancesRequest runRequest;
    runRequest.WithInstanceType(vmType)
        .WithPlacement(getPlacement())
        .WithImageId(config_.amiId)
        .WithMinCount(1)
        .WithMaxCount(1);

    const auto vmTypeName = Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(runRequest.GetInstanceType());

    auto result = client_->RunInstances(runRequest);
    if (!result.IsSuccess()) {
        return Result<AwsVmInfo>::Failure<AllocationException>(toString(
            "Failed to start ec2 vm ",
            vmTypeName,
            " based on ami ",
            runRequest.GetImageId(),
            " in region ",
            runRequest.GetPlacement().GetAvailabilityZone(),
            ": ",
            result.GetError().GetMessage()));
    }

    const auto& vms = result.GetResult().GetInstances();
    if (vms.empty()) {
        return Result<AwsVmInfo>::Failure<AllocationException>(toString(
            "Failed to create ec2 vm ", vmTypeName, " based on ami ", runRequest.GetImageId(), ": empty vms list"));
    }

    const auto vmId = vms[0].GetInstanceId();
    INFO() << "Successfully created ec2 vm " << vmId << " with type: " << vmTypeName << " based on ami "
           << runRequest.GetImageId();

    return Result{AwsVmInfo{.type = vmType, .id = vmId}};
}

Result<void> AwsCloudClient::startInstance(const AwsVmInfo& vmInfo)
{
    Aws::EC2::Model::StartInstancesRequest startRequest;
    startRequest.AddInstanceIds(vmInfo.id);

    auto result = client_->StartInstances(startRequest);
    if (result.IsSuccess()) {
        INFO() << "Successfully started ec2 vm " << startRequest.GetInstanceIds()[0];
        return Result<void>::Success();
    }

    return Result<void>::Failure<AllocationException>(
        toString("Failed to start vm: ", vmInfo.id, ": ", result.GetError().GetMessage()));
}

Result<void> AwsCloudClient::terminate(const TerminationPendingVmInfo& vmInfo) noexcept
{
    return terminate(vmInfo.cloudVmId);
}

Result<void> AwsCloudClient::terminate(const CloudVmId& cloudVmId)
{
    Aws::EC2::Model::TerminateInstancesRequest request;
    request.AddInstanceIds(cloudVmId);

    const auto result = client_->TerminateInstances(request);
    if (result.IsSuccess()) {
        INFO() << "Successfully terminated ec2 vm: " << request.GetInstanceIds()[0];
        return Result<void>::Success();
    }
    return Result<void>::Failure<AllocationException>(
        toString("Failed to terminate ec2 vm ", request.GetInstanceIds()[0], result.GetError().GetMessage()));
}

} // namespace vm_scheduler
