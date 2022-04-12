#pragma once

#include "libs/allocator/include/cloud_client.h"

#include <gmock/gmock.h>

namespace vm_scheduler::testing {

class CloudClientMock : public CloudClient {
public:
    using CloudClient::CloudClient;

    MOCK_METHOD(Result<AllocatedVmInfo>, allocate, (const SlotCapacity& slot), (noexcept, override));
    MOCK_METHOD(Result<void>, terminate, (const TerminationPendingVmInfo& vmInfo), (noexcept, override));
};

} // namespace vm_scheduler::testing
