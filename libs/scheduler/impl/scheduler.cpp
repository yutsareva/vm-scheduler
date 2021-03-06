#include "libs/scheduler/include/scheduler.h"
#include "libs/scheduler/include/create_vm_assigner.h"
#include "libs/scheduler/include/vm_assigner.h"

#include "libs/common/include/errors.h"

#include <libs/common/include/log.h>

namespace vm_scheduler {

Scheduler::Scheduler(
    BackendId id,
    TaskStorage* taskStorage,
    std::vector<SlotCapacity> possibleSlots,
    std::shared_ptr<DistributedLock> distLock)
    : id_(std::move(id))
    , taskStorage_(taskStorage)
    , config_(createSchedulerConfig())
    , possibleSlots_(std::move(possibleSlots))
    , distLock_(std::move(distLock))
{
    if (possibleSlots_.empty()) {
        throw RuntimeException("Possible slots array is empty");
    }
    if (!std::is_sorted(possibleSlots_.begin(), possibleSlots_.end())) {
        throw RuntimeException("Possible slots array is not sorted");
    }
    if (distLock_ != nullptr) {
        distLock_->start();
    }
}

void Scheduler::schedule() noexcept
{
    std::optional<size_t> lockNumber = std::nullopt;
    if (distLock_ != nullptr) {
        lockNumber = distLock_->lockNumber();
        if (!lockNumber) {
            ERROR() << "Lock is not acquired.";
            return;
        }
    }
    const auto planIdResult =
        taskStorage_->startScheduling(id_, config_.schedulingInterval, lockNumber);
    if (planIdResult.IsFailure()) {
        ERROR() << "Failed to start new scheduling iteration: "
                << what(planIdResult.ErrorRefOrThrow());
        return;
    }
    INFO() << "New plan id: " << planIdResult.ValueRefOrThrow();

    auto currentStateResult = taskStorage_->getCurrentState();
    if (currentStateResult.IsFailure()) {
        ERROR() << "Failed to get current state: "
                << what(currentStateResult.ErrorRefOrThrow());
        return;
    }
    INFO() << "Current state: " << currentStateResult.ValueRefOrThrow();

    const auto vmAssigner = createVmAssigner(
        config_.vmAssignerType,
        std::move(currentStateResult).ValueOrThrow(),
        config_.complexVmAssignerConfig,
        possibleSlots_);
    const auto stateChange = vmAssigner->assign();
    INFO() << "State change: " << stateChange;

    const auto commitResult = taskStorage_->commitPlanChange(
        stateChange, planIdResult.ValueRefOrThrow());
    if (commitResult.IsFailure()) {
        ERROR() << "Failed to commit scheduling result: "
                << what(commitResult.ErrorRefOrThrow());
        return;
    }
}

} // namespace vm_scheduler
