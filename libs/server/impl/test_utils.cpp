#include "libs/server/impl/test_utils.h"

#include <chrono>
#include <thread>

namespace vm_scheduler::testing {

using namespace std::chrono_literals;

proto::Task generateProtoTask()
{
    proto::JobLimits job_limits;
    job_limits.set_memory_mb(1);
    job_limits.set_cpu_cores(2);
    proto::TaskLimits limits;
    *limits.mutable_job_limits() = std::move(job_limits);
    limits.set_execution_s(3);

    proto::Task task;
    *task.mutable_limits() = std::move(limits);
    task.set_settings("{}");
    task.set_job_count(2);
    task.set_image_version("version");
    task.set_client_id("client id");
    const std::vector<JobOptions> jobOptions = {"{}", "{}"};
    *task.mutable_job_options() = {jobOptions.begin(), jobOptions.end()};

    return task;
}

std::unique_ptr<proto::PublicApiScheduler::Stub> createPublicApiSchedulerStub()
{
    const auto serverConfig = createServerConfig();
    auto channel = grpc::CreateChannel(
        serverConfig.address.c_str(), grpc::InsecureChannelCredentials());
    return proto::PublicApiScheduler::NewStub(channel);
}

proto::TaskAdditionResult addTask(const proto::Task& task)
{
    proto::TaskAdditionResult protoTaskAdditionResult;
    grpc::ClientContext context;

    auto stub = createPublicApiSchedulerStub();
    grpc::Status additionStatus =
        stub->addTask(&context, task, &protoTaskAdditionResult);
    assert(additionStatus.error_code() == grpc::StatusCode::OK);
    return protoTaskAdditionResult;
}

proto::TaskExecutionResult waitTaskForComplete(
    proto::TaskId taskId, const std::chrono::seconds& timeout)
{
    auto start = std::chrono::steady_clock::now();
    auto stub = createPublicApiSchedulerStub();

    while (std::chrono::steady_clock::now() - start < timeout) {
        grpc::ClientContext context;
        proto::TaskState taskState;

        grpc::Status status = stub->getTaskState(&context, taskId, &taskState);
        assert(status.error_code() == grpc::StatusCode::OK);
        if (taskState.status() == proto::TaskStatus::TASK_COMPLETED) {
            break;
        }
        std::this_thread::sleep_for(1s);
    }
    grpc::ClientContext taskResultContext;
    proto::TaskExecutionResult taskResult;
    grpc::Status taskResultStatus =
        stub->getTaskResult(&taskResultContext, taskId, &taskResult);
    assert(taskResultStatus.error_code() == grpc::StatusCode::OK);

    return taskResult;
}

} // namespace vm_scheduler::testing
