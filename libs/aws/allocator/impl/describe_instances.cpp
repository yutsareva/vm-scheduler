#include "libs/aws/allocator/impl/describe_instances.h"
#include "libs/aws/allocator/impl/helpers.h"

#include <libs/common/include/stringify.h>

#include <aws/ec2/model/DescribeInstancesRequest.h>

namespace vm_scheduler {

namespace {

Aws::EC2::Model::DescribeInstancesRequest createRequest(const Tags& tags)
{
    Aws::Vector<Aws::EC2::Model::Filter> filters;
    filters.reserve(tags.size());

    for (const auto& [key, value]: tags) {
        Aws::EC2::Model::Filter filter;
        filter = filter.WithName("tag:" + key).WithValues({value});
        filters.emplace_back(std::move(filter));
    }

    Aws::EC2::Model::Filter statusFilter;
    statusFilter = statusFilter.WithName("instance-state-name")
                       .WithValues({"pending", "running"});
    filters.emplace_back(std::move(statusFilter));

    Aws::EC2::Model::DescribeInstancesRequest request;
    request = request.WithFilters(filters);

    return request;
}

} // anonymous namespace

Result<AllocatedVmInfos> describeInstances(
    Aws::EC2::EC2Client& client, const Tags& tags)
{
    auto request = createRequest(tags);
    AllocatedVmInfos vmInfos;

    Aws::EC2::Model::DescribeInstancesOutcome result;
    do {
        result = client.DescribeInstances(request);
        if (!result.IsSuccess()) {
            return Result<AllocatedVmInfos>::Failure(toString(
                "Failed to describe instances: ", result.GetError().GetMessage()));
        }

        for (const auto& reservation: result.GetResult().GetReservations()) {
            const auto& instances = reservation.GetInstances();
            std::transform(
                instances.begin(),
                instances.end(),
                std::inserter(vmInfos, vmInfos.begin()),
                [](const Aws::EC2::Model::Instance& info) -> AllocatedVmInfo {
                    return convertAwsVmInfoToVmInfo({
                        .type = info.GetInstanceType(),
                        .id = info.GetInstanceId(),
                    });
                });
        }

        request.SetNextToken(result.GetResult().GetNextToken());
    } while (!result.GetResult().GetNextToken().empty());

    return Result{std::move(vmInfos)};
}

} // namespace vm_scheduler
