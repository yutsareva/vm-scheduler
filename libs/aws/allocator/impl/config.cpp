#include "libs/aws/allocator/include/config.h"
#include "libs/aws/allocator/impl/describe_instance_types.h"

#include <libs/common/include/env.h>
#include <libs/common/include/log.h>

namespace vm_scheduler {

namespace {

// custom AMI with preinstalled docker and aws cli for ARM
const std::string DEFAULT_AWS_EC2_AMI_ID = "ami-0aaa7d236c8b7d9b9";
const std::string DEFAULT_AWS_INSTANCE_CLASS = "c6g.";
const std::string DEFAULT_AWS_INSTANCE_TAGS = "owner:vms-test";
const std::string DEFAULT_AWS_VM_TOKEN_PREFIX = "vms-prod";
const std::string DEFAULT_AGENT_DOCKER_IMAGE_VERSION = "latest";
constexpr size_t DEFAULT_USE_SPOT = 0;

std::unordered_map<std::string, std::string> getVmTags()
{
    // TODO: parse tags from env var

    return {
        {"owner", "vms-prod"},
    };
}
} // anonymous namespace

AwsClientConfig createAwsClientConfig()
{
    return AwsClientConfig{
        .accessKeyId = getFromEnvOrThrow("AWS_EC2_ACCESS_KEY_ID"),
        .secretKey = getFromEnvOrThrow("AWS_EC2_SECRET_KEY"),
    };
}

auto InstanceTypeInfo::operator<(const InstanceTypeInfo& other) const
{
    return capacity < other.capacity;
}

std::ostream& operator<<(std::ostream& out, const InstanceTypeInfos& vmTypeInfos)
{
    for (const auto& vmTypeInfo: vmTypeInfos) {
        out << "[type: "
            << Aws::EC2::Model::InstanceTypeMapper::GetNameForInstanceType(
                   vmTypeInfo.type)
            << ", "
            << "capacity: " << vmTypeInfo.capacity << "], ";
    }
    return out;
}

AwsInstancesConfig createAwsInstancesConfig(Aws::EC2::EC2Client& client)
{
    const auto instanceClass =
        getFromEnvOrDefault("VMS_AWS_INSTANCE_CLASS", DEFAULT_AWS_INSTANCE_CLASS);
    auto instanceTypeInfos =
        describeInstanceTypes(client, instanceClass).ValueOrThrow();
    std::sort(instanceTypeInfos.begin(), instanceTypeInfos.end());

    INFO() << "VM types: " << instanceTypeInfos;

    return {
        .amiId = getFromEnvOrDefault("AWS_EC2_AMI_ID", DEFAULT_AWS_EC2_AMI_ID),
        .vmTypes = std::move(instanceTypeInfos),
        .vmTags = getVmTags(),
        .tokenPrefix = getFromEnvOrDefault(
            "VMS_AWS_VM_TOKEN_PREFIX", DEFAULT_AWS_VM_TOKEN_PREFIX),
        .agentDockerImageVersion = getFromEnvOrDefault(
            "VMS_AGENT_DOCKER_IMAGE_VERSION", DEFAULT_AGENT_DOCKER_IMAGE_VERSION),
        .vmsAddress = getFromEnvOrThrow("VMS_ADDRESS"),
        .creds =
            AgentEcrCredentials{
                .accessKeyId = getFromEnvOrThrow("VMS_AGENT_ECR_ACCESS_KEY_ID"),
                .secretKey = getFromEnvOrThrow("VMS_AGENT_ECR_SECRET_KEY"),
            },
        .useSpot = bool(getFromEnvOrDefault("VMS_USE_SPOT", DEFAULT_USE_SPOT)),
    };
}

} // namespace vm_scheduler
