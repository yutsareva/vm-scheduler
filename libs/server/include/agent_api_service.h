#pragma once

#include <services/agent_api.grpc.pb.h>

#include <google/protobuf/empty.pb.h>
#include <grpcpp/grpcpp.h>

namespace vm_scheduler {

class GrpcServer;

class AgentApiSchedulerService final : public proto::AgentApiScheduler::Service {
public:
    explicit AgentApiSchedulerService(GrpcServer& grpcServer);

    grpc::Status getAssignedJobs(
        grpc::ServerContext* context, const proto::VmId* request, proto::AssignedJobs* response) override;

    grpc::Status getJobToLaunch(
        grpc::ServerContext* context, const proto::LaunchRequest* request, proto::JobToLaunch* response) override;

    grpc::Status updateJobState(
        grpc::ServerContext* context, const proto::ExecutionJobState* request, google::protobuf::Empty* response) override;

private:
    GrpcServer& grpcServer_;
};

} // namespace vm_scheduler
