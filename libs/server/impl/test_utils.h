#include "libs/server/include/grpc_server.h"

#include <memory>

namespace vm_scheduler::testing {

proto::Task generateProtoTask();

std::unique_ptr<proto::PublicApiScheduler::Stub> createPublicApiSchedulerStub();

proto::TaskAdditionResult addTask(const proto::Task& task = generateProtoTask());

proto::TaskExecutionResult waitTaskForComplete(
    proto::TaskId taskId, const std::chrono::seconds& timeout);

} // namespace vm_scheduler::testing
