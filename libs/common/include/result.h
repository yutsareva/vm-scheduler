#pragma once

#include "libs/common/include/errors.h"

#include <exception>
#include <optional>
#include <variant>

namespace vm_scheduler {

template<typename T>
class [[nodiscard]] Result {
public:
    explicit Result(T value) : result_(std::move(value)) { }
    explicit Result(std::exception_ptr error) : result_(std::move(error)) { }

    static Result Success(T value) { return {std::move(value)}; }

    static Result Failure(std::exception_ptr error) { return Result{std::move(error)}; }

    template<typename TException>
    static Result Failure(const std::string& errorString)
    {
        return Result{std::make_exception_ptr(TException(errorString))};
    }

    static Result Failure(const std::string& errorString)
    {
        return Result{std::make_exception_ptr(RuntimeException(errorString))};
    }

    bool IsFailure() const { return std::holds_alternative<std::exception_ptr>(result_); }

    bool IsSuccess() const { return std::holds_alternative<T>(result_); }

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
        throw RuntimeException("Tried to extract error from Result with value");
    }

    const std::exception_ptr& ErrorRefOrThrow() const
    {
        if (IsFailure()) {
            return std::get<std::exception_ptr>(result_);
        }
        throw RuntimeException("Tried to extract error from Result with value");
    }

    template<typename TError>
    bool holdsErrorType() const
    {
        if (IsSuccess()) {
            return false;
        }
        try {
            std::rethrow_exception(ErrorRefOrThrow());
        } catch (const TError& ex) {
            return true;
        } catch (const std::exception& ex) {
            return false;
        }
    }

private:
    std::variant<std::exception_ptr, T> result_;
};

template<>
class [[nodiscard]] Result<void> {
public:
    static Result Success() { return {}; }

    explicit Result(std::exception_ptr error) : error_(std::move(error)) { }

    static Result Failure(std::exception_ptr error) { return Result{std::move(error)}; }

    template<typename TException>
    static Result Failure(const std::string& errorString)
    {
        return Result{std::make_exception_ptr(TException(errorString))};
    }

    bool IsFailure() const { return error_.has_value(); }

    bool IsSuccess() const { return !IsFailure(); }
    std::exception_ptr&& ErrorOrThrow() &&
    {
        if (IsFailure()) {
            return std::move(error_).value();
        }
        throw RuntimeException("Tried to extract error from Result with value");
    }
    const std::exception_ptr& ErrorRefOrThrow() const
    {
        if (IsFailure()) {
            return error_.value();
        }
        throw RuntimeException("Tried to extract error from Result with value");
    }

    template<typename TError>
    bool holdsErrorType() const
    {
        if (IsSuccess()) {
            return false;
        }
        try {
            std::rethrow_exception(ErrorRefOrThrow());
        } catch (const TError& ex) {
            return true;
        } catch (const std::exception& ex) {
            return false;
        }
    }

private:
    Result() = default;
    std::optional<std::exception_ptr> error_;
};

} // namespace vm_scheduler
