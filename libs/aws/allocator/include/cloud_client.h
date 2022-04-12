#pragma once

#include "libs/aws/allocator/include/config.h"
#include <libs/allocator/include/cloud_client.h>

#include <aws/ec2/EC2Client.h>
#include <memory>

namespace vm_scheduler {

struct AwsVmInfo {
    Aws::EC2::Model::InstanceType type;
    Aws::String id;
};

class AwsCloudClient : public CloudClient {
public:
    AwsCloudClient(AwsEc2Config config = createEc2Config());
    ~AwsCloudClient();

    Result<AllocatedVmInfo> allocate(const SlotCapacity& slot) noexcept override;
    Result<void> terminate(const TerminationPendingVmInfo& vmInfo) noexcept override;

private:
    Result<AwsVmInfo> allocate(const Aws::EC2::Model::InstanceType& vmType);
    Result<AwsVmInfo> createInstance(const Aws::EC2::Model::InstanceType& vmType);
    Result<void> startInstance(const AwsVmInfo& vmInfo);
    Result<void> terminate(const CloudVmId& cloudVmId);

private:
    AwsEc2Config config_;
    std::unique_ptr<Aws::EC2::EC2Client> client_;
    Aws::SDKOptions options_;
};
} // namespace vm_scheduler
