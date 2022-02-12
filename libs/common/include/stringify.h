#pragma once

#include <string>
#include <sstream>

namespace vm_scheduler {

template<typename... P>
std::string toString(const P&... params)
{
    std::stringstream stream;

    (stream << ... << params);

    return stream.str();
}

} // namespace vm_scheduler
