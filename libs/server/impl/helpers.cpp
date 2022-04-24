#include <libs/server/impl/helpers.h>

#include <libs/common/include/errors.h>
#include <libs/common/include/stringify.h>

#include <libs/common/include/log.h>

#include <exception>

namespace vm_scheduler {

namespace {

constexpr auto INTERNAL_ERROR_MSG_PREFIX = "Internal error. Incident id: ";

} // anonymous namespace

grpc::Status grpcInternalErrorStatus(
    const std::exception_ptr& ex, const std::string& msg)
{
    const auto errorUuid = "123-345-5656"; // TODO: generate uuid
    ERROR() << errorUuid << msg << what(ex);
    const auto errorMsg = toString(INTERNAL_ERROR_MSG_PREFIX, errorUuid);
    return grpc::Status(grpc::StatusCode::INTERNAL, errorMsg);
}

proto::JobStatus jobStatusToProto(const JobStatus jobStatus)
{
    switch (jobStatus) {
        case JobStatus::Queued:
        case JobStatus::Scheduled:
            return proto::JobStatus::JOB_QUEUED;
        case JobStatus::Running:
            return proto::JobStatus::JOB_RUNNING;
        case JobStatus::Completed:
            return proto::JobStatus::JOB_COMPLETED;
        case JobStatus::Cancelling:
        case JobStatus::Cancelled:
            return proto::JobStatus::JOB_CANCELLED;
        case JobStatus::Error:
        case JobStatus::InternalError:
            return proto::JobStatus::JOB_FAILED;
    }
}

} // namespace vm_scheduler
