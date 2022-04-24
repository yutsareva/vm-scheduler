#pragma once

#include "libs/task_registry/include/config.h"

#include <libs/allocator/include/allocator.h>
#include <libs/common/include/types.h>
#include <libs/concurrency/include/background_thread.h>
#include <libs/failure_detector/include/failure_detector.h>
#include <libs/scheduler/include/scheduler.h>
#include <libs/server/include/grpc_server.h>
#include <libs/task_storage/include/task_storage.h>

#include <memory>
#include <string>

namespace vm_scheduler {

class TaskRegistry {
public:
    TaskRegistry(
        const Config& config,
        std::unique_ptr<TaskStorage>&& taskStorage,
        std::unique_ptr<CloudClient>&& cloudClient);

private:
    BackendId id_;

    std::unique_ptr<TaskStorage> taskStorage_;
    Allocator allocator_;
    Scheduler scheduler_;
    FailureDetector failureDetector_;
    GrpcServer grpcServer_;

    std::unique_ptr<BackgroundThread> allocationThread_;
    std::unique_ptr<BackgroundThread> terminationThread_;
    std::unique_ptr<BackgroundThread> schedulingThread_;
    std::unique_ptr<BackgroundThread> monitorThread_;
};

} // namespace vm_scheduler
