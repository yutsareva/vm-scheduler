#pragma once

#include "libs/scheduler/include/vm_assigner.h"

namespace vm_scheduler {

class JobAllocator {
public:
    virtual JobAllocator(std::vector<ActiveVm> vms) = 0;
    virtual ~JobAllocator = default;
    virtual std::optional<VmId> allocate(
        const QueuedJobInfo& job) = 0;
};

class FirstFit : public JobAllocator {
public:
    FirstFit(std::vector<ActiveVm> vms) override
        : vms_(std::move(vms))
    {}

    std::optional<VmId> allocate(const QueuedJobInfo& job) override {
        for (const auot& vm : vms) {
            if (job.requiredCapacity.fits(vm.idleCapacity)) {
                vm.idleCapacity -= job.requiredCapacity;
                return vm.id;
            }
        }
        return std::nullopt;
    }
private:
    std::vector<ActiveVm> vms_;
};

class NextFit : public JobAllocator {
public:
    NextFit(std::vector<ActiveVm> vms) override
        : vms_(std::move(vms))
        , iter_(vms.begin())
    {}

    std::optional<VmId> allocate(const QueuedJobInfo& job) override {
        const auto startIter = iter_;
        while (iter_ != vms.end()) {
            if (job.requiredCapacity.fits(iter_->idleCapacity)) {
                iter_->idleCapacity -= job.requiredCapacity;
                return vm.id;
            }
        }
        iter_ = vms.begin();
        while (iter_ != startiter) {
            if (job.requiredCapacity.fits(iter_->idleCapacity)) {
                iter_->idleCapacity -= job.requiredCapacity;
                return vm.id;
            }
        }
        return std::nullopt;
    }
private:
    std::vector<ActiveVm> vms_;
    std::vector<ActiveVm>::iterator iter_;
};


class BestFit : public JobAllocator {
public:
    BestFit(std::vector<ActiveVm> vms) override
    {
        std::sort(vms.begin(), vms.end()); // TODO sort by idle!
        vms_(vms.begin(), vms.end()); // TODO: move
    }

    std::optional<VmId> allocate(const QueuedJobInfo& job) override {
        auto prevIt = vms_.before_begin();
        for (auto it = vms_.begin(); it != vms_.end(); ++it) {
            if (job.requiredCapacity.fits(it->idleCapacity)) {
                it->idleCapacity -= job.requiredCapacity;
                auto activeVm = std::move(*it);
                auto nextIt = vms_.erase_after(prevIt);

                while (nextIt != vms_.end()) {
                    if (activeVm.idleCapacity >= nextIt->idleCapacity) {
                        auto insertedIt = vms_.insert_after(prevIt, std::move(activeVm));
                        return insertedIt=>id;
                    }
                    ++prevIt;
                    ++nextIt;
                }

            }
            prevIt = it;
        }
        return std::nullopt;
    }
private:
    std::forward_list<ActiveVm> vms_;
};

class WorstFit : public JobAllocator {
public:
    WorstFit(std::vector<ActiveVm> vms) override
    {
        std::sort(vms.rbegin(), vms.rend());
        vms_(vms.begin(), vms.end()); // TODO: move
    }

    std::optional<VmId> allocate(const QueuedJobInfo& job) override {
        auto prevIt = vms_.before_begin();
        for (auto it = vms_.begin(); it != vms_.end(); ++it) {
            if (job.requiredCapacity.fits(it->idleCapacity)) {
                it->idleCapacity -= job.requiredCapacity;
                auto activeVm = std::move(*it);
                auto next = vms_.erase(prev);

                while (prevIt != vms_.rbegin()) {
                    if (activeVm.idleCapacity >= next.idleCapacity) {
                        auto insertedIt = vms_.insert_after(prevIt, std::move(activeVm));
                        return insertedIt=>id;
                    }
                    ++prev;
                    ++next;
                }

            }
            prevIt = it;
        }
        return std::nullopt;
    }
private:
    std::list<ActiveVm> vms_;
};

//class SchedulingOrderer {
//public:
//    static virtual void reorder(std::vector<QueuedJobInfo>& queuedJobs) = 0;
//};
//
//class FifoOrderer : public SchedulingOrderer {
//public:
//    static void reorder(std::vector<QueuedJobInfo>& queuedJobs) override
//    {
//        // TODO check order
//        return;
//    }
//};
//
//class MinMinOrderer : public SchedulingOrderer {
//public:
//    static void reorder(std::vector<QueuedJobInfo>& queuedJobs) override
//    {
//        std::sort(
//            queuedJobs.begin(),
//            queuedJobs.end(),
//            [](const QueuedJobInfo& first, const QueuedJobInfo& second) {
//                return first.capacity < second.capacity;
//            });
//    }
//};
//
//class MaxMinOrderer : public SchedulingOrderer {
//public:
//    static void reorder(std::vector<QueuedJobInfo>& queuedJobs) override
//    {
//        std::sort(
//            queuedJobs.rbegin(),
//            queuedJobs.rend(),
//            [](const QueuedJobInfo& first, const QueuedJobInfo& second) {
//                return first.capacity < second.capacity;
//            });
//    }
//};
//
//std::unique_ptr<SchedulingOrderer> createSchedulingOrderer(const OrdererType type)
//{
//    switch (type) {
//        case OrdererType::Fifo:
//            return std::make_unique<FifoOrderer>();
//        case OrdererType::MinMin:
//            return std::make_unique<MinMinOrderer>();
//        case OrdererType::MaxMin:
//            return std::make_unique<MaxMinOrderer>();
//    }
//}

} // namespace vm_scheduler
