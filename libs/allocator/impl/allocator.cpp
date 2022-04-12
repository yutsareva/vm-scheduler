#include "libs/allocator/include/allocator.h"
#include "libs/common/include/errors.h"

#include <libs/common/include/log.h>
#include <libs/common/include/stringify.h>

#include <memory>

namespace vm_scheduler {

Allocator::Allocator(TaskStorage* taskStorage, std::unique_ptr<CloudClient>&& cloudClient)
    : taskStorage_(taskStorage), cloudClient_(std::move(cloudClient)), config_(createAllocatorConfig()){};

void Allocator::allocate() noexcept
{
    auto vmsToAllocate = taskStorage_->getVmsToAllocate(config_.maxVmAllocationCount);
    if (vmsToAllocate.IsFailure()) {
        ERROR() << "Failed to get vms for allocation: " << what(std::move(vmsToAllocate).ErrorOrThrow());
        return;
    }

    std::vector<VmId> unallocatedVms;
    for (const auto& vm: vmsToAllocate.ValueRefOrThrow()) {
        INFO() << "Allocating VM: " << vm.id;
        auto allocationResult = cloudClient_->allocate(vm.capacity);
        if (allocationResult.IsSuccess()) {
            const auto& cloudVmInfo = allocationResult.ValueRefOrThrow();
            auto saveAllocationResult = taskStorage_->saveVmAllocationResult(vm.id, cloudVmInfo);

            if (saveAllocationResult.IsFailure()) {
                ERROR() << "Failed to save allocation result for vm with id " << vm.id << ", cloud_vm_id "
                        << cloudVmInfo.id << ", cloud_vm_type " << cloudVmInfo.type << ": "
                        << what(std::move(saveAllocationResult).ErrorOrThrow());
                // TODO: in a bright future Failure Detector will process allocated but not recorded VMs
            }
        } else {
            ERROR() << "Failed to allocate vm for id " << vm.id << ", capacity: " << vm.capacity.cpu.count() << "cpu, "
                    << vm.capacity.ram.count() << "MB: " << what(std::move(allocationResult).ErrorOrThrow());
            unallocatedVms.push_back(vm.id);
        }
    }

    if (!unallocatedVms.empty()) {
        auto returnResult = taskStorage_->rollbackUnallocatedVmsState(
            unallocatedVms, config_.common.vmRestartAttemptCount, config_.common.jobRestartAttemptCount);

        if (returnResult.IsFailure()) {
            ERROR() << "Failed to restart or terminate unallocated vms: " << joinSeq(", ", unallocatedVms) << ": "
                    << what(std::move(returnResult).ErrorOrThrow());
        }
    }
}

void Allocator::terminate() noexcept
{
    auto vmsToTerminate = taskStorage_->getVmsToTerminate(config_.maxVmTerminationCount);
    if (vmsToTerminate.IsFailure()) {
        ERROR() << "Failed to get vms for termination: " << what(std::move(vmsToTerminate).ErrorOrThrow());
        return;
    }

    std::vector<VmId> unterminatedVms;
    for (const auto& vm: vmsToTerminate.ValueRefOrThrow()) {
        INFO() << "Terminating VM: " << vm.id;

        auto terminationResult = cloudClient_->terminate(vm);

        if (terminationResult.IsSuccess()) {
            auto changeStatusResult = taskStorage_->saveVmTerminationResult(vm.id);
            if (changeStatusResult.IsFailure()) {
                ERROR() << "Failed to set status 'terminated' for vm with id " << vm.id
                        << ": " << what(std::move(changeStatusResult).ErrorOrThrow());

                // TODO: in a bright future Failure Detector will process terminated but not recorded VMs
            }
        } else {
            ERROR() << "Failed to terminate vm for id " << vm.id << ", cloud vm id: " << vm.cloudVmId << ": "
                    << what(std::move(terminationResult).ErrorOrThrow());
            unterminatedVms.push_back(vm.id);
        }
    }

    if (!unterminatedVms.empty()) {
        auto returnResult = taskStorage_->returnUnterminatedVms(unterminatedVms);

        if (returnResult.IsFailure()) {
            ERROR() << "Failed to return unterminated vms to initial status: " << JoinSeq(", ", unterminatedVms) << ": "
                    << what(std::move(returnResult).ErrorOrThrow());
            // TODO: in a bright future Failure Detector will process VMs in wrong 'terminating' status
        }
    }
}

} // namespace vm_scheduler
