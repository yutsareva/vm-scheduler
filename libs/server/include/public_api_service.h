#pragma once

#include <services/public_api.grpc.pb.h>

#include <google/protobuf/empty.pb.h>
#include <grpcpp/grpcpp.h>

namespace vm_scheduler {

class GrpcServer;

class PublicApiSchedulerService final :
    public proto::PublicApiScheduler::Service {
public:
    explicit PublicApiSchedulerService(GrpcServer& grpcServer);

    grpc::Status addTask(
        grpc::ServerContext* context,
        const proto::Task* request,
        proto::TaskAdditionResult* response) override;
    grpc::Status getTaskState(
        grpc::ServerContext* context,
        const proto::TaskId* request,
        proto::TaskState* response) override;
    grpc::Status getTaskResult(
        grpc::ServerContext* context,
        const proto::TaskId* request,
        proto::TaskExecutionResult* response) override;
    grpc::Status cancelRunningJobs(
        grpc::ServerContext* context,
        const proto::TaskId* request,
        google::protobuf::Empty* response) override;
    grpc::Status healthCheck(
        grpc::ServerContext* context,
        const google::protobuf::Empty* request,
        google::protobuf::Empty* response) override;

private:
    GrpcServer& grpcServer_;
};

} // namespace vm_scheduler
