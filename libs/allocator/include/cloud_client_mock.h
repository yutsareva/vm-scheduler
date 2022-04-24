#pragma once

#include "libs/allocator/include/cloud_client.h"

#include <gmock/gmock.h>

namespace vm_scheduler::testing {

class CloudClientMock : public CloudClient {
public:
    using CloudClient::CloudClient;

    MOCK_METHOD(
        Result<AllocatedVmInfo>,
        allocate,
        (const VmId vmId, const SlotCapacity& slot),
        (noexcept, override));
    MOCK_METHOD(
        Result<void>, terminate, (const CloudVmId& vmInfo), (noexcept, override));
    MOCK_METHOD(
        Result<AllocatedVmInfos>, getAllAllocatedVms, (), (noexcept, override));
};

} // namespace vm_scheduler::testing
