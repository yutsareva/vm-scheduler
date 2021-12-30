#include "aws_allocator.h"

#include <aws/ec2/model/RunInstancesRequest.h>

namespace vm_scheduler::allocator {

void AwsAllocator::allocate(const vm_scheduler::allocator::Slot& slot)
{

}

void AwsAllocator::deallocate(const Slot& slot)
{

}

void AwsAllocator::createInstance(const Slot& slot)
{
    Aws::EC2::Model::RunInstancesRequest runRequest;
    runRequest = runRequest
        .SetImageId(ami_id)
        .SetInstanceType(Aws::EC2::Model::InstanceType::t1_micro)
        .SetMinCount(1)
        .SetMaxCount(1);

    auto result = client_.RunInstances(runRequest);
    if (!result.IsSuccess()) {
        std::cout << "Failed to start ec2 instance " << instanceName <<
            " based on ami " << ami_id << ":" <<
            result.GetError().GetMessage() << std::endl;
        return;
    }


}

} // namespace vm_scheduler::allocator
