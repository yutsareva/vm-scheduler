#include "libs/server/impl/errors.h"
#include "libs/server/impl/helpers.h"
#include "libs/server/include/grpc_server.h"

#include <libs/common/include/stringify.h>
#include <libs/common/include/types.h>
#include <libs/state/include/task.h>

#include <structures/task.pb.h>
#include <structures/task_execution_result.pb.h>

#include <libs/common/include/log.h>

#include <rapidjson/document.h>

#include <grpcpp/grpcpp.h>
#include <iterator>

namespace vm_scheduler {

namespace {

Result<void> validateJson(
    const std::string& jsonStr, const std::string& fieldName) noexcept
{
    rapidjson::Document doc;
    const rapidjson::ParseResult ok = doc.Parse(jsonStr.data(), jsonStr.size());
    if (!ok) {
        return Result<void>::Failure<InputValidationException>(toString(
            "Field `", fieldName, "` validation failed. ",
            "Invalid json at position: ", ok.Offset()));
    }
    return Result<void>::Success();
}

Result<void> validateProtoTaskParameters(const proto::Task& task) noexcept
{
    if (static_cast<size_t>(task.job_options_size()) != task.job_count()) {
        return Result<void>::Failure<InputValidationException>(toString(
            "`job_options` size (", task.job_options_size(),
            ") must be equal to `job_count`(", task.job_count(), ")."));
    }
    auto settingsValidationResult = validateJson(task.settings(), "settings");
    if (settingsValidationResult.IsFailure()) {
        return Result<void>::Failure(
            std::move(settingsValidationResult).ErrorOrThrow());
    }
    for (size_t i = 0; i < static_cast<size_t>(task.job_options_size()); ++i) {
        auto optionsValidationResult =
            validateJson(task.job_options()[i], toString("options[", i, "]"));
        if (optionsValidationResult.IsFailure()) {
            return Result<void>::Failure(
                std::move(optionsValidationResult).ErrorOrThrow());
        }
    }
    return Result<void>::Success();
}

Result<TaskParameters> taskParametersFromProto(const proto::Task& task) noexcept
{
    auto result = validateProtoTaskParameters(task);
    if (result.IsFailure()) {
        return Result<TaskParameters>::Failure(std::move(result).ErrorOrThrow());
    }

    return Result{TaskParameters{
        .requiredCapacity =
            {
                .cpu = CpuCores(task.limits().job_limits().cpu_cores()),
                .ram = MegaBytes(task.limits().job_limits().memory_mb()),
            },
        .estimation = std::chrono::seconds(task.limits().execution_s()),
        .jobCount = task.job_count(),
        .settings = task.settings(),
        .imageVersion = task.image_version(),
        .clientId = task.client_id(),
        .jobOptions = {task.job_options().begin(), task.job_options().end()},
    }};
}

template<typename TProtoJobState>
proto::TaskStatus jobStatesToTaskStatus(
    const std::vector<TProtoJobState>& protoJobStates)
{
    if (std::find_if(
            protoJobStates.begin(),
            protoJobStates.end(),
            [](const TProtoJobState& protoJobState) {
                return protoJobState.status() == proto::JobStatus::JOB_RUNNING;
            }) != protoJobStates.end()) {
        return proto::TaskStatus::TASK_RUNNING;
    }
    if (std::find_if(
            protoJobStates.begin(),
            protoJobStates.end(),
            [](const TProtoJobState& protoJobState) {
                return protoJobState.status() == proto::JobStatus::JOB_QUEUED;
            }) != protoJobStates.end()) {
        return proto::TaskStatus::TASK_QUEUED;
    }
    return proto::TaskStatus::TASK_COMPLETED;
}

proto::TaskState protoTaskStateFromJobStates(const JobStates& jobStates)
{
    std::vector<proto::JobState> protoJobStates;
    protoJobStates.reserve(jobStates.size());
    std::transform(
        jobStates.begin(),
        jobStates.end(),
        std::back_inserter(protoJobStates),
        [](const JobState& jobState) -> proto::JobState {
            proto::JobState protoJobState;
            protoJobState.set_status(jobStatusToProto(jobState.status));
            return protoJobState;
        });
    proto::TaskState protoTaskState;
    *protoTaskState.mutable_job_states() = {
        protoJobStates.begin(), protoJobStates.end()};
    protoTaskState.set_status(jobStatesToTaskStatus(protoJobStates));
    return protoTaskState;
}

proto::TaskExecutionResult protoTaskExecutionResultFromJobStates(
    const JobStates& jobStates)
{
    std::vector<proto::JobExecutionResult> protoJobResults;
    protoJobResults.reserve(jobStates.size());
    std::transform(
        jobStates.begin(),
        jobStates.end(),
        std::back_inserter(protoJobResults),
        [](const JobState& jobState) -> proto::JobExecutionResult {
            proto::JobExecutionResult protoJobResult;
            protoJobResult.set_status(jobStatusToProto(jobState.status));
            if (jobState.resultUrl) {
                protoJobResult.set_result_url(jobState.resultUrl->c_str());
            }
            return protoJobResult;
        });
    proto::TaskExecutionResult protoTaskResult;
    *protoTaskResult.mutable_job_results() = {
        protoJobResults.begin(), protoJobResults.end()};
    protoTaskResult.set_status(jobStatesToTaskStatus(protoJobResults));
    return protoTaskResult;
}

} // anonymous namespace

PublicApiSchedulerService::PublicApiSchedulerService(GrpcServer& grpcServer)
    : grpcServer_(grpcServer)
{ }

grpc::Status PublicApiSchedulerService::addTask(
    grpc::ServerContext*,
    const proto::Task* protoTask,
    proto::TaskAdditionResult* protoResult)
{
    const auto taskParameters = taskParametersFromProto(*protoTask);
    if (taskParameters.IsFailure()) {
        INFO() << "Failed to add task. Invalid request: "
               << what(taskParameters.ErrorRefOrThrow());
        return grpc::Status(
            grpc::StatusCode::INVALID_ARGUMENT,
            what(taskParameters.ErrorRefOrThrow()));
    }
    const auto taskResult =
        grpcServer_.taskStorage_->addTask(taskParameters.ValueRefOrThrow());
    if (taskResult.IsFailure()) {
        return grpcInternalErrorStatus(
            taskParameters.ErrorRefOrThrow(), "Failed to add task: ");
    }
    protoResult->mutable_task_id()->set_value(taskResult.ValueRefOrThrow().taskId);
    return grpc::Status::OK;
};

grpc::Status PublicApiSchedulerService::getTaskState(
    grpc::ServerContext*,
    const proto::TaskId* taskId,
    proto::TaskState* protoTaskState)
{
    const auto jobStates =
        grpcServer_.taskStorage_->getJobStates(taskId->value());
    if (jobStates.IsFailure()) {
        return grpcInternalErrorStatus(
            jobStates.ErrorRefOrThrow(),
            "getTaskState: Unexpected error while querying job statuses: ");
    }
    if (jobStates.ValueRefOrThrow().empty()) {
        return grpc::Status(
            grpc::StatusCode::NOT_FOUND,
            toString("Task ", taskId->value(), " not found."));
    }

    *protoTaskState = protoTaskStateFromJobStates(jobStates.ValueRefOrThrow());
    return grpc::Status::OK;
};

grpc::Status PublicApiSchedulerService::getTaskResult(
    grpc::ServerContext*,
    const proto::TaskId* taskId,
    proto::TaskExecutionResult* protoResult)
{
    const auto jobStates =
        grpcServer_.taskStorage_->getJobStates(taskId->value());
    if (jobStates.IsFailure()) {
        return grpcInternalErrorStatus(
            jobStates.ErrorRefOrThrow(),
            "getTaskResult: Unexpected error while querying job statuses: ");
    }
    if (jobStates.ValueRefOrThrow().empty()) {
        return grpc::Status(
            grpc::StatusCode::NOT_FOUND,
            toString("Task ", taskId->value(), " not found."));
    }

    *protoResult =
        protoTaskExecutionResultFromJobStates(jobStates.ValueRefOrThrow());
    return grpc::Status::OK;
};

grpc::Status PublicApiSchedulerService::cancelRunningJobs(
    grpc::ServerContext*, const proto::TaskId* taskId, google::protobuf::Empty*)
{
    const auto cancellationResult =
        grpcServer_.taskStorage_->cancelTask(taskId->value());
    if (cancellationResult.IsFailure()) {
        return grpcInternalErrorStatus(
            cancellationResult.ErrorRefOrThrow(),
            "cancelRunningJobs: Unexpected error while cancelling task: ");
    }
    return grpc::Status::OK;
};

grpc::Status PublicApiSchedulerService::healthCheck(
    grpc::ServerContext*, const google::protobuf::Empty*, google::protobuf::Empty*)
{
    return grpc::Status::OK;
}

} // namespace vm_scheduler
