#pragma once

#include "libs/scheduler/include/vm_assigner.h"


namespace vm_scheduler {

enum class OrdererType {
    Fifo /*"fifo"*/,
    MinMin /*"minmin"*/,
    MaxMin /*"maxmin"*/,
};

OrdererType ordererTypeFromString(const std::string& s) {
    const static std::unordered_map<std::string, OrdererType> map = {
        {"fifo", OrdererType::Fifo},
        {"minmin", OrdererType::MinMin},
        {"maxmin", OrdererType::MaxMin},
    };
    return map.at(s);
}

struct ComplexVmAssignerConfig {
    OrdererType ordererType;
};

ComplexVmAssignerConfig createComplexVmAssignerConfig()
{
    return ComplexVmAssignerConfig{
        .ordererType = ordererTypeFromString(getFromEnvOrDefault(
            "VMS_ORDERER_TYPE", "fifo")),

    };
}

class ComplexVmAssigner : public VmAssigner {
public:
//    using VmAssigner::VmAssigner;
    ComplexVmAssigner(const ComplexVmAssignerConfig& config);
    StateChange assign() noexcept override;
private:
    SchedulingOrderer orderer_;
};

} // namespace vm_scheduler
