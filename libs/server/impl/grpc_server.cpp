#include "libs/server/include/grpc_server.h"

#include "libs/common/include/stringify.h"

#include <libs/common/include/log.h>

namespace vm_scheduler {

GrpcServer::GrpcServer(ServerConfig config, TaskStorage* taskStorage)
    : taskStorage_(taskStorage), publicApiSchedulerService_(*this), agentApiSchedulerService_(*this)
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort(config.address, grpc::InsecureServerCredentials());
    server_ =
        builder.RegisterService(&publicApiSchedulerService_).RegisterService(&agentApiSchedulerService_).BuildAndStart();
    if (!server_) {
        throw std::runtime_error(toString("Failed to start gRPC server on ", config.address));
    }
    INFO() << "Started gRPC server on " << config.address;
}

} // namespace vm_scheduler
