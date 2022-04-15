#pragma once

#include <libs/server/include/agent_api_service.h>
#include <libs/server/include/config.h>
#include <libs/server/include/public_api_service.h>

#include <libs/task_storage/include/task_storage.h>

#include <grpcpp/grpcpp.h>

namespace vm_scheduler {

class GrpcServer {
public:
    GrpcServer(ServerConfig config, TaskStorage* taskStorage);

    friend class AgentApiSchedulerService;
    friend class PublicApiSchedulerService;

private:
    TaskStorage* taskStorage_;
    PublicApiSchedulerService publicApiSchedulerService_;
    AgentApiSchedulerService agentApiSchedulerService_;
    std::unique_ptr<grpc::Server> server_;
};

} // namespace vm_scheduler
