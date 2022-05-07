#pragma once

#include <exception>
#include <string>

namespace vm_scheduler {

class RuntimeException : public std::exception {
public:
    explicit RuntimeException(std::string errorMsg)
        : errorMsg_(std::move(errorMsg))
    { }
    const char* what() const noexcept override { return errorMsg_.c_str(); }

private:
    std::string errorMsg_;
};

const char* what(const std::exception_ptr& exPtr);

} // namespace vm_scheduler
