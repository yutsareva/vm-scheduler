#include "libs/scheduler/include/scheduler.h"
#include "libs/scheduler/include/create_vm_assigner.h"
#include "libs/scheduler/include/vm_assigner.h"
#include "libs/common/include/errors.h"

#include <libs/lcommon/log.h>


namespace vm_scheduler {

Scheduler::Scheduler(BackendId id, TaskStorage* taskStorage)
    : id_(std::move(id))
    , taskStorage_(taskStorage)
    , config_(createSchedulerConfig())
          {
          };

void Scheduler::schedule() noexcept
{
    auto planIdResult = taskStorage_->startScheduling(id_, config_.schedulingInterval);
    if (planIdResult.IsFailure()) {
        ERROR() << "Failed to start new scheduling iteration: " << what(std::move(planIdResult).ErrorOrThrow());
        return;
    }

    auto currentStateResult = taskStorage_->getCurrentState();
    if (currentStateResult.IsFailure()) {
        ERROR() << "Failed to get current state: " << what(std::move(currentStateResult).ErrorOrThrow());
        return;
    }

    const auto vmAssigner = createVmAssigner(config_.vmAssignerType, currentStateResult.ValueRefOrThrow());
    const auto stateChange = vmAssigner->assign();

    auto commitResult = taskStorage_->commitPlanChange(stateChange, planIdResult.ValueRefOrThrow());
    if (commitResult.IsFailure()) {
        ERROR() << "Failed to commit scheduling result: " << what(std::move(commitResult).ErrorOrThrow());
        return;
    }
}

} // namespace vm_scheduler
