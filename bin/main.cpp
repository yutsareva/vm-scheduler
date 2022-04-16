#include <libs/task_registry/include/task_registry.h>
#include <libs/aws/allocator/include/cloud_client.h>
#include <libs/db/include/pg_task_storage.h>

#include <libs/postgres/include/helpers.h>
#include <libs/common/include/log.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <signal.h>

namespace vms = vm_scheduler;

std::atomic<bool> isStopping{false};
std::mutex mutex;
std::condition_variable isStopped;

void sigterm(int sigint)
{
    if (isStopping) {
        INFO() << "Terminated";
        std::exit(sigint);
    }
    INFO() << "Stopping VM Scheduler...";
    isStopping.store(true);
}

void installSignalHandlers()
{
    struct sigaction sa;
    sa.sa_handler = sigterm;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, nullptr);
}

int main()
{
    installSignalHandlers();
    auto pgPool = vms::pg::createPool();
    auto taskRegistry = std::make_unique<vms::TaskRegistry>(
        vms::createConfig(),
        std::make_unique<vms::PgTaskStorage>(std::move(pgPool)),
        std::make_unique<vms::AwsCloudClient>());

    std::unique_lock lock(mutex);
    while (!isStopping) {
        isStopped.wait(lock);
    }
    INFO() << "Shutting down...";
    taskRegistry.reset();
    INFO() << "Server is shutdown.";
    return 0;
}
