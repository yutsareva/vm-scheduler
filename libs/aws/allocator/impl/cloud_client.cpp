#include "libs/aws/allocator/include/cloud_client.h"
#include "libs/aws/allocator/impl/describe_instances.h"
#include "libs/aws/allocator/impl/errors.h"
#include "libs/aws/allocator/impl/helpers.h"
#include "libs/aws/allocator/impl/select_instance_type.h"
#include "libs/aws/allocator/include/config.h"

#include <libs/common/include/base64.h>
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
    // TODO: in a bright future Allocator should allocate instances in different availability zones
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
        Aws::Auth::AWSCredentials{config.accessKeyId, config.secretKey, ""},
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
    auto vmInfo = createInstance(vmToken, vmType, vmId);
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

std::string getUserData(
    const std::string& agentDockerImageVersion,
    const std::string& vmsAddress,
    const AgentEcrCredentials& creds,
    const VmId vmId)
{
    return base64Encode(toString(
        "#!/bin/bash\n",
        "set -e -x\n",
        "aws configure set aws_access_key_id ",
        creds.accessKeyId,
        "\n",
        "aws configure set aws_secret_access_key {",
        creds.secretKey,
        "\n",
        "aws configure set default.region us-east-2\n",
        "aws configure set default.output json\n",
        "sudo aws ecr get-login-password --region us-east-2 | ",
        "sudo docker login --username AWS --password-stdin 545868914688.dkr.ecr.us-east-2.amazonaws.com\n",
        "sudo docker run -e VMS_ADDRESS=",
        vmsAddress,
        "-e VM_ID=",
        vmId,
        " --network host 545868914688.dkr.ecr.us-east-2.amazonaws.com/vm-agent:",
        agentDockerImageVersion,
        "\n"));
}

Aws::EC2::Model::RunInstancesRequest createRunRequest(
    const std::string& vmToken,
    const AwsInstancesConfig& config,
    const Aws::EC2::Model::InstanceType& vmType,
    const VmId vmId,
    const bool useSpot)
{
    Aws::EC2::Model::TagSpecification tagSpecs;
    tagSpecs = tagSpecs.WithResourceType(Aws::EC2::Model::ResourceType::instance);

    Aws::Vector<Aws::EC2::Model::Tag> awsTags;
    for (const auto& [key, value]: config.vmTags) {
        Aws::EC2::Model::Tag tag;
        tag.WithKey(key).WithValue(value);
        awsTags.emplace_back(std::move(tag));
    }
    tagSpecs.WithTags(awsTags);

    Aws::EC2::Model::RunInstancesRequest request;

    Aws::EC2::Model::IamInstanceProfileSpecification iamSpec;
    iamSpec.WithName("vmsAgentExecutorRole");

    request.WithInstanceType(vmType)
        .WithPlacement(getPlacement())
        .WithImageId(config.amiId)
        .WithTagSpecifications({tagSpecs})
        .WithClientToken(vmToken) // makes allocation idempotent
        .WithUserData(getUserData(
            config.agentDockerImageVersion, config.vmsAddress, config.creds, vmId))
        .WithIamInstanceProfile(iamSpec)
        .WithKeyName("vms_instance")
        .WithMinCount(1)
        .WithMaxCount(1);

    if (useSpot) {
        Aws::EC2::Model::InstanceMarketOptionsRequest marketOptions;
        marketOptions
            .WithMarketType(Aws::EC2::Model::MarketType::spot);
        // The default spot price is the On-Demand price.

        request.WithInstanceMarketOptions(marketOptions);
    }

    return request;
}

} // anonymous namespace

Result<AwsVmInfo> AwsCloudClient::createInstance(
    const std::string& vmToken,
    const Aws::EC2::Model::InstanceType& vmType,
    const VmId vmId) noexcept
{
    const auto runRequest = createRunRequest(vmToken, vmConfig_, vmType, vmId, vmConfig_.useSpot);

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

    const auto awsVmId = vms[0].GetInstanceId();
    INFO() << "Successfully created ec2 vm " << awsVmId
           << " with type: " << vmTypeName << " based on ami "
           << runRequest.GetImageId();

    return Result{AwsVmInfo{.type = vmType, .id = awsVmId}};
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

std::vector<SlotCapacity> AwsCloudClient::getPossibleSlots() noexcept
{
    std::vector<SlotCapacity> possibleSlots;
    for (const auto& vmType: vmConfig_.vmTypes) {
        possibleSlots.push_back(vmType.capacity);
    }
    return possibleSlots;
}

} // namespace vm_scheduler
