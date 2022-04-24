#include "libs/server/include/agent_api_service.h"
#include "libs/server/impl/helpers.h"
#include "libs/server/include/grpc_server.h"

#include <libs/common/include/errors.h>
#include <libs/common/include/stringify.h>
#include <libs/state/include/errors.h>
#include <libs/state/include/task.h>

#include <libs/common/include/log.h>

#include <grpcpp/grpcpp.h>
#include <unordered_set>

namespace vm_scheduler {

namespace {

void setAssignedJobs(
    proto::AssignedJobs* protoAssignedJobs,
    const std::vector<AssignedJob>& assignedJobs)
{
    for (const auto job: assignedJobs) {
        proto::AssignedJob* protojob = protoAssignedJobs->add_jobs();
        protojob->mutable_id()->set_value(job.id);
        protojob->set_status(jobStatusToProto(job.status));
    }
}

void setJobToLaunch(proto::JobToLaunch* protoJob, const JobToLaunch& job)
{
    protoJob->mutable_id()->set_value(job.id);
    protoJob->set_image_version(job.imageVersion.c_str());
    protoJob->mutable_job_limits()->set_memory_mb(job.capacityLimits.ram.count());
    protoJob->mutable_job_limits()->set_cpu_cores(job.capacityLimits.cpu.count());
    protoJob->set_task_settings(job.taskSettings.c_str());
    protoJob->set_job_options(job.jobOptions.c_str());
}

JobStatus protoJobStatusToJobStatus(const proto::JobStatus jobStatus)
{
    switch (jobStatus) {
        case proto::JobStatus::JOB_QUEUED:
            return JobStatus::Queued;
        case proto::JobStatus::JOB_RUNNING:
            return JobStatus::Running;
        case proto::JobStatus::JOB_COMPLETED:
            return JobStatus::Completed;
        case proto::JobStatus::JOB_CANCELLED:
            return JobStatus::Cancelled;
        case proto::JobStatus::JOB_FAILED:
            return JobStatus::Error;
    }
}

JobState protoExecutionJobStateToJobState(
    const proto::ExecutionJobState& protoExecutionJobState)
{
    return {
        .status = protoJobStatusToJobStatus(
            protoExecutionJobState.job_result().status()),
        .resultUrl = protoExecutionJobState.job_result().result_url(),
    };
}

const std::unordered_set<JobStatus>& getAllowedAgentUpdateJobStatuses()
{
    static const std::unordered_set<JobStatus> allowedAgentUpdateJobStatuses = {
        JobStatus::Running,
        JobStatus::Completed,
        JobStatus::Cancelled,
        JobStatus::Error,
    };
    return allowedAgentUpdateJobStatuses;
}

} // anonymous namespace

AgentApiSchedulerService::AgentApiSchedulerService(GrpcServer& grpcServer)
    : grpcServer_(grpcServer)
{ }

grpc::Status AgentApiSchedulerService::getAssignedJobs(
    grpc::ServerContext*, const proto::VmId* vmId, proto::AssignedJobs* protoJobIds)
{
    const auto assignedJobs =
        grpcServer_.taskStorage_->getAssignedJobs(vmId->value());
    if (assignedJobs.IsFailure()) {
        return grpcInternalErrorStatus(
            assignedJobs.ErrorRefOrThrow(),
            "getAssignedJobs: Unexpected error while querying job statuses: ");
    }
    setAssignedJobs(protoJobIds, assignedJobs.ValueRefOrThrow());
    return grpc::Status::OK;
}

grpc::Status AgentApiSchedulerService::getJobToLaunch(
    grpc::ServerContext*,
    const proto::LaunchRequest* protoLaunchRequest,
    proto::JobToLaunch* protoJob)
{
    const auto jobToLaunch = grpcServer_.taskStorage_->getJobToLaunch(
        protoLaunchRequest->vm_id().value(),
        protoLaunchRequest->job_id().value());

    if (jobToLaunch.IsFailure()) {
        if (jobToLaunch.holdsErrorType<JobNotFoundException>()) {
            INFO() << "getJobToLaunch: " << what(jobToLaunch.ErrorRefOrThrow());
            return grpc::Status(
                grpc::StatusCode::NOT_FOUND,
                toString(
                    "Job ", protoLaunchRequest->job_id().value(), " not found."));
        }
        return grpcInternalErrorStatus(
            jobToLaunch.ErrorRefOrThrow(),
            "getJobToLaunch: Unexpected error while getting job to launch: ");
    }

    setJobToLaunch(protoJob, jobToLaunch.ValueRefOrThrow());
    return grpc::Status::OK;
}

grpc::Status AgentApiSchedulerService::updateJobState(
    grpc::ServerContext*,
    const proto::ExecutionJobState* protoExecutionJobState,
    google::protobuf::Empty*)
{
    const auto jobState =
        protoExecutionJobStateToJobState(*protoExecutionJobState);
    const auto& allowedForUpdateStatuses = getAllowedAgentUpdateJobStatuses();
    if (!allowedForUpdateStatuses.contains(jobState.status)) {
        return grpc::Status(
            grpc::StatusCode::INVALID_ARGUMENT,
            toString(
                "Status ", toString(jobState.status),
                " is not allowed to be set by agent"));
    }

    const auto result = grpcServer_.taskStorage_->updateJobState(
        protoExecutionJobState->vm_id().value(),
        protoExecutionJobState->job_id().value(),
        jobState);

    if (result.IsFailure()) {
        INFO() << "updateJobState: " << what(result.ErrorRefOrThrow());
        if (result.holdsErrorType<JobNotFoundException>()) {
            return grpc::Status(
                grpc::StatusCode::NOT_FOUND,
                toString(
                    "Job ", protoExecutionJobState->job_id().value(),
                    " not found."));
        } else if (result.holdsErrorType<JobCancelledException>()) {
            return grpc::Status(
                grpc::StatusCode::CANCELLED,
                toString(
                    "Job ", protoExecutionJobState->job_id().value(),
                    " was cancelled."));
        }
        return grpcInternalErrorStatus(
            result.ErrorRefOrThrow(),
            "updateJobState: Unexpected error while getting job to launch: ");
    }

    return grpc::Status::OK;
}

} // namespace vm_scheduler
