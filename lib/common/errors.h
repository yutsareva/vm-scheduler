#pragma once

#include <string>

namespace vm_scheduler {

class RuntimeException : public std::exception {
public:
    explicit RuntimeException(std::string errorMsg) : errorMsg_(std::move(errorMsg)) { }
    const char* what() const noexcept override
    {
        return errorMsg_.c_str();
    }

private:
    std::string errorMsg_;
};

class AllocationException : public RuntimeException {
public:
    explicit AllocationException(const std::string& errorMsg) : RuntimeException(errorMsg) { }
};
} // namespace vm_scheduler
