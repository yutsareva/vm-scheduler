#include "vm_selector.h"

#include <aws/ec2/model/DescribeInstanceTypeOfferingsRequest.h>
#include <aws/ec2/model/Filter.h>

namespace vm_scheduler::vm_selector {

TaskToVm AwsVmSelector::select(
        const std::vector<IdleSlot>& idleSlots,
        const std::vector<task::TaskSlot>& requiredCapacities)
{
    // https://docs.aws.amazon.com/AWSEC2/latest/APIReference/API_DescribeInstanceTypeOfferings.html
    Aws::EC2::Model::DescribeInstanceTypeOfferingsRequest request;
    Aws::EC2::Model::Filter filter;
    // https://sdk.amazonaws.com/cpp/api/LATEST/class_aws_1_1_e_c2_1_1_model_1_1_describe_instance_type_offerings_request.html
    filter.WithName("location")
        .WithValues(config_.region);
    request.AddFilters(std::move(filter));

    const auto result = client_.DescribeInstanceTypeOfferings(request);
    if (result.IsSuccess()) {
        const auto vmTypesOfferings = result.GetInstanceTypeOfferings();
    }
    return {};
}

} // namespace vm_scheduler::vm_selector
