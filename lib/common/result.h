#pragma once

#include <exception>
#include <variant>


namespace vm_scheduler {

template<typename T>
class [[nodiscard]] Result {
public:
    bool HasError() const
    {
        return std::holds_alternative<std::exception_ptr>(result_);
    }

    T ValueOrThrow() &&
    {
        if (HasError()) {
            std::rethrow_exception(std::get<std::exception_ptr>(result_));
        }
        return std::get<T>(result_);
    }

private:
    std::variant<std::exception_ptr, T> result_;
};

} // namespace vm_scheduler
