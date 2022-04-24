#include "libs/aws/allocator/include/cloud_client.h"
#include "libs/aws/allocator/impl/describe_instances.h"
#include "libs/aws/allocator/impl/errors.h"
#include "libs/aws/allocator/impl/helpers.h"
#include "libs/aws/allocator/impl/select_instance_type.h"
#include "libs/aws/allocator/include/config.h"

#include <libs/common/include/env.h>

#include <libs/common/include/log.h>

#include <aws/core/auth/AWSCredentials.h>
#include <aws/ec2/model/RunInstancesRequest.h>
#include <aws/ec2/model/StartInstancesRequest.h>
#include <aws/ec2/model/TerminateInstancesRequest.h>

#include <algorithm>

namespace vm_scheduler {

namespace {

Aws::EC2::Model::Placement getPlacement()
{
    // TODO: in a bright future Allocater should allocate instances in different availability zones
    // for jobs from one task
    Aws::EC2::Model::Placement placement;
    placement.SetAvailabilityZone("us-east-2a");
    return placement;
}

} // anonymous namespace

AwsCloudClient::AwsCloudClient(const AwsClientConfig& config) : options_()
{
    Aws::InitAPI(options_);
    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.verifySSL = false;
    client_ = std::make_unique<Aws::EC2::EC2Client>(
        Aws::Auth::AWSCredentials{
            config.accessKeyId, config.secretKey, config.sessionToken},
        clientConfig);

    vmConfig_ = createAwsInstancesConfig(*client_);
}

AwsCloudClient::~AwsCloudClient()
{
    Aws::ShutdownAPI(options_);
}

Aws::EC2::Model::InstanceType getInstanceTypeForSlot(
    const InstanceTypeInfos& vmTypeInfos, const SlotCapacity& slot)
{
    const auto it = std::lower_bound(
        vmTypeInfos.begin(),
        vmTypeInfos.end(),
        slot,
        [](const InstanceTypeInfo& vmTypeInfo, const SlotCapacity& slot) {
            return vmTypeInfo.capacity < slot;
        });
    if (it == vmTypeInfos.end()) {
        ERROR() << "Appropriate instance type not found for slot " << slot
                << ". "
                << "Took the largest one: "
                << Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(
                       vmTypeInfos.rbegin()->type);
        return vmTypeInfos.rbegin()->type;
    }
    return it->type;
}

Aws::EC2::Model::InstanceType AwsCloudClient::instanceTypeForSlot(
    const SlotCapacity& slot) const noexcept
{
    return getInstanceTypeForSlot(vmConfig_.vmTypes, slot);
}

Result<AllocatedVmInfo> AwsCloudClient::allocate(
    const VmId vmId, const SlotCapacity& slot) noexcept
{
    const auto vmType = instanceTypeForSlot(slot);
    auto result = allocate(vmId, vmType);
    if (result.IsSuccess()) {
        return Result{convertAwsVmInfoToVmInfo(std::move(result).ValueOrThrow())};
    }
    return Result<AllocatedVmInfo>::Failure(std::move(result).ErrorOrThrow());
}

Result<AwsVmInfo> AwsCloudClient::allocate(
    const VmId vmId, const Aws::EC2::Model::InstanceType& vmType) noexcept
{
    const auto vmToken = toString(vmConfig_.tokenPrefix, "-", vmId);
    auto vmInfo = createInstance(vmToken, vmType);
    if (vmInfo.IsFailure()) {
        return vmInfo;
    }

    auto startInstanceResult = startInstance(vmInfo.ValueRefOrThrow());
    if (startInstanceResult.IsFailure()) {
        ERROR() << "Failed to start vm " << vmInfo.ValueRefOrThrow().id
                << ", terminating...";

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

namespace {

Aws::EC2::Model::RunInstancesRequest createRunRequest(
    const std::string& vmToken,
    const Tags& tags,
    const Aws::EC2::Model::InstanceType& vmType,
    const std::string& amiId)
{
    Aws::EC2::Model::TagSpecification tagSpecs;
    tagSpecs = tagSpecs.WithResourceType(Aws::EC2::Model::ResourceType::instance);

    Aws::Vector<Aws::EC2::Model::Tag> awsTags;
    for (const auto& [key, value]: tags) {
        Aws::EC2::Model::Tag tag;
        tag.WithKey(key).WithValue(value);
        awsTags.emplace_back(std::move(tag));
    }
    tagSpecs.WithTags(awsTags);

    Aws::EC2::Model::RunInstancesRequest request;
    request.WithInstanceType(vmType)
        .WithPlacement(getPlacement())
        .WithImageId(amiId)
        .WithTagSpecifications({tagSpecs})
        .WithClientToken(vmToken) // for making allocation idempotent
        .WithMinCount(1)
        .WithMaxCount(1);

    return request;
}

} // anonymous namespace

Result<AwsVmInfo> AwsCloudClient::createInstance(
    const std::string& vmToken,
    const Aws::EC2::Model::InstanceType& vmType) noexcept
{
    const auto runRequest =
        createRunRequest(vmToken, vmConfig_.vmTags, vmType, vmConfig_.amiId);

    const auto vmTypeName =
        Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(
            runRequest.GetInstanceType());

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
            "Failed to create ec2 vm ",
            vmTypeName,
            " based on ami ",
            runRequest.GetImageId(),
            ": empty vms list"));
    }

    const auto vmId = vms[0].GetInstanceId();
    INFO() << "Successfully created ec2 vm " << vmId
           << " with type: " << vmTypeName << " based on ami "
           << runRequest.GetImageId();

    return Result{AwsVmInfo{.type = vmType, .id = vmId}};
}

Result<void> AwsCloudClient::startInstance(const AwsVmInfo& vmInfo) noexcept
{
    Aws::EC2::Model::StartInstancesRequest startRequest;
    startRequest.AddInstanceIds(vmInfo.id);

    auto result = client_->StartInstances(startRequest);
    if (result.IsSuccess()) {
        INFO() << "Successfully started ec2 vm "
               << startRequest.GetInstanceIds()[0];
        return Result<void>::Success();
    }

    return Result<void>::Failure<AllocationException>(toString(
        "Failed to start vm: ", vmInfo.id, ": ", result.GetError().GetMessage()));
}

Result<void> AwsCloudClient::terminate(const CloudVmId& cloudVmId) noexcept
{
    Aws::EC2::Model::TerminateInstancesRequest request;
    request.AddInstanceIds(cloudVmId);

    const auto result = client_->TerminateInstances(request);
    if (result.IsSuccess()) {
        INFO() << "Successfully terminated ec2 vm: "
               << request.GetInstanceIds()[0];
        return Result<void>::Success();
    }
    return Result<void>::Failure<AllocationException>(toString(
        "Failed to terminate ec2 vm ",
        request.GetInstanceIds()[0],
        result.GetError().GetMessage()));
}

Result<AllocatedVmInfos> AwsCloudClient::getAllAllocatedVms() noexcept
{
    return describeInstances(*client_, vmConfig_.vmTags);
}

} // namespace vm_scheduler
