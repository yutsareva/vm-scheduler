#pragma once

#include <libs/state/include/task.h>
#include <structures/job.pb.h>

#include <grpcpp/grpcpp.h>

#include <exception>
#include <string>

namespace vm_scheduler {

grpc::Status grpcInternalErrorStatus(const std::exception_ptr& ex, const std::string& msg);

proto::JobStatus jobStatusToProto(const JobStatus jobStatus);

} // namespace vm_scheduler
