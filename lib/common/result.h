#pragma once

#include <exception>
#include <variant>

namespace vm_scheduler {

template<typename T>
class [[nodiscard]] Result {
public:
    Result(T&& value) { result_ = std::move(value); }
    Result(std::exception_ptr error)
    {
        result_ = std::move(error);
    }

    static Result Success(T&& value) {
        return {std::move(value)};
    }

    static Result Failure(std::exception_ptr error)
    {
        return {std::move(error)};
    }

    static Result Failure(const std::string& errorString)
    {
        return {std::make_exception_ptr(std::runtime_error(errorString))};
    }

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
