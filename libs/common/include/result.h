#pragma once

#include <exception>
#include <optional>
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

    template<typename TException>
    static Result Failure(const std::string& errorString)
    {
        return {std::make_exception_ptr(TException(errorString))};
    }

    template<typename TException>
    static Result Failure(const std::string& errorString, std::exception_ptr nestedError)
    {
        try {
            throw(TException(errorString));
        } catch (const TException& outerError) {
            try {
                std::throw_with_nested(nestedError);
            } catch (const std::exception&) {
                return Failure(std::current_exception());
            }
        }
    }

    static Result Failure(const std::string& errorString)
    {
        return {std::make_exception_ptr(RuntimeException(errorString))};
    }

    bool IsFailure() const
    {
        return std::holds_alternative<std::exception_ptr>(result_);
    }

    bool IsSuccess() const
    {
        return std::holds_alternative<T>(result_);
    }

    T ValueOrThrow() &&
    {
        if (IsFailure()) {
            std::rethrow_exception(std::get<std::exception_ptr>(std::move(result_)));
        }
        return std::get<T>(std::move(result_));
    }

    const T& ValueRefOrThrow() const
    {
        if (IsFailure()) {
            std::rethrow_exception(std::get<std::exception_ptr>(result_));
        }
        return std::get<T>(result_);
    }

    std::exception_ptr&& ErrorOrThrow() &&
    {
        if (IsFailure()) {
            return std::get<std::exception_ptr>(std::move(result_));
        }
        throw RuntimeException("tried to extract error from Result with value");
    }

private:
    std::variant<std::exception_ptr, T> result_;
};

template <>
class [[nodiscard]] Result<void> {
public:
    static Result Success() {
        return {};
    }

    Result(std::exception_ptr error)
    {
        error_ = std::move(error);
    }

    static Result Failure(std::exception_ptr error)
    {
        return {std::move(error)};
    }

    template<typename TException>
    static Result Failure(const std::string& errorString)
    {
        return {std::make_exception_ptr(TException(errorString))};
    }

    bool IsFailure() const
    {
        return error_.has_value();
    }

    bool IsSuccess() const
    {
        return !IsFailure();
    }
    std::exception_ptr&& ErrorOrThrow() &&
    {
        if (IsFailure()) {
            return std::move(error_).value();
        }
        throw RuntimeException("tried to extract error from Result with value");
    }
private:
    Result() = default;
    std::optional<std::exception_ptr> error_;
};

} // namespace vm_scheduler
