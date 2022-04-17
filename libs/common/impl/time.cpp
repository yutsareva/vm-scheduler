#include "libs/common/include/time.h"

namespace vm_scheduler {

std::_Put_time<char> getCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    return std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
}

} // namespace vm_scheduler
