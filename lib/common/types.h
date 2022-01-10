#pragma once

#include <compare>
#include <cstddef>

namespace vm_scheduler {

class GigaBytes {
public:
    constexpr explicit GigaBytes(size_t value) : value_(value) { }
    auto operator<=>(const GigaBytes&) const = default;

private:
    size_t value_;
};

constexpr GigaBytes operator""_GB(unsigned long long value)
{
    return GigaBytes(value);
}

class CpuCores {
public:
    constexpr explicit CpuCores(size_t value) : value_(value) { }
    auto operator<=>(const CpuCores&) const = default;

private:
    size_t value_;
};

constexpr CpuCores operator""_cores(unsigned long long value)
{
    return CpuCores(value);
}


} // namespace vm_scheduler
