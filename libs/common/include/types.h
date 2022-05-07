#pragma once

#include <compare>
#include <cstddef>
#include <string>

namespace vm_scheduler {

class MegaBytes {
public:
    constexpr explicit MegaBytes(size_t value) : value_(value) { }
    auto operator<=>(const MegaBytes&) const = default;
    size_t count() const { return value_; }

private:
    size_t value_;
};

constexpr MegaBytes operator""_MB(unsigned long long value)
{
    return MegaBytes(value);
}

class CpuCores {
public:
    constexpr explicit CpuCores(size_t value) : value_(value) { }
    auto operator<=>(const CpuCores&) const = default;
    size_t count() const { return value_; }

private:
    size_t value_;
};

constexpr CpuCores operator""_cores(unsigned long long value)
{
    return CpuCores(value);
}

using BackendId = std::string;

} // namespace vm_scheduler
