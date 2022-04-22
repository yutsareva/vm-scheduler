#include "libs/aws/allocator/impl/describe_instance_types.h"

#include <libs/common/include/stringify.h>
#include <libs/common/include/types.h>

#include <aws/ec2/model/DescribeInstanceTypesRequest.h>
#include <aws/ec2/model/Filter.h>
#include <aws/ec2/model/InstanceTypeInfo.h>

#include <iterator>

namespace vm_scheduler {

Result<InstanceTypeInfos> describeInstanceTypes(
    Aws::EC2::EC2Client& client, const std::string& instanceClass)
{
    Aws::EC2::Model::Filter filter;
    filter = filter.WithName("instance-type").WithValues({instanceClass + "*"});

    Aws::EC2::Model::DescribeInstanceTypesRequest request;
    request = request.WithFilters({filter});

    InstanceTypeInfos instanceTypeInfos;

    while (true) {
        auto result = client.DescribeInstanceTypes(request);
        if (!result.IsSuccess()) {
            return Result<InstanceTypeInfos>::Failure(toString(
                "Failed to describe instance types: ", result.GetError().GetMessage()));
        }

        const auto& awsInstanceTypeInfos = result.GetResult().GetInstanceTypes();

        std::transform(
            awsInstanceTypeInfos.begin(),
            awsInstanceTypeInfos.end(),
            std::back_inserter(instanceTypeInfos),
            [](const Aws::EC2::Model::InstanceTypeInfo& info) -> InstanceTypeInfo
            {
                return {
                    .type = info.GetInstanceType(),
                    .capacity = {
                        .cpu = CpuCores(static_cast<size_t>(info.GetVCpuInfo().GetDefaultCores())),
                        .ram = MegaBytes(static_cast<size_t>(info.GetMemoryInfo().GetSizeInMiB())),
                    },
                };
            });

        if (result.GetResult().GetNextToken().empty()) {
            return Result{std::move(instanceTypeInfos)};
        }
        request.SetNextToken(result.GetResult().GetNextToken());
    }
}

} // namespace vm_scheduler
