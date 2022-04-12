#include "libs/common/include/errors.h"

#include <sstream>

namespace vm_scheduler {

const char* what(const std::exception_ptr& exPtr)
{
    try {
        std::rethrow_exception(exPtr);
    } catch (const std::exception& ex) {
        return ex.what();
    }
}

} // namespace vm_scheduler
