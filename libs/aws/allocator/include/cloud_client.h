#pragma once

#include "libs/aws/allocator/impl/helpers.h"
#include "libs/aws/allocator/include/config.h"
#include <libs/allocator/include/cloud_client.h>

#include <aws/ec2/EC2Client.h>

#include <memory>

namespace vm_scheduler {

class AwsCloudClient : public CloudClient {
public:
    AwsCloudClient(const AwsClientConfig& config = createAwsClientConfig());
    ~AwsCloudClient();

    Result<AllocatedVmInfo> allocate(
        const VmId vmId, const SlotCapacity& slot) noexcept override;
    Result<void> terminate(const CloudVmId& cloudVmId) noexcept override;
    Result<AllocatedVmInfos> getAllAllocatedVms() noexcept override;

private:
    Result<AwsVmInfo> allocate(
        const VmId vmId, const Aws::EC2::Model::InstanceType& vmType) noexcept;
    Result<AwsVmInfo> createInstance(
        const std::string& vmToken,
        const Aws::EC2::Model::InstanceType& vmType) noexcept;
    Result<void> startInstance(const AwsVmInfo& vmInfo) noexcept;
    Aws::EC2::Model::InstanceType instanceTypeForSlot(
        const SlotCapacity& slot) const noexcept;

private:
    std::unique_ptr<Aws::EC2::EC2Client> client_;
    AwsInstancesConfig vmConfig_;
    Aws::SDKOptions options_;
};

} // namespace vm_scheduler
