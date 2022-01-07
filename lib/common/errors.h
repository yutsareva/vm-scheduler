#pragma once

#include <string>

namespace vm_scheduler {

class RuntimeException : std::runtime_error {
public:
    explicit RuntimeException(const std::string& errorMsg) : std::runtime_error(errorMsg) { }
};

class AllocationException : RuntimeException {
public:
    explicit AllocationException(const std::string& errorMsg) : RuntimeException(errorMsg) { }
};
} // namespace vm_scheduler
