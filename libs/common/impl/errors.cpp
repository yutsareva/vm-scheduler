#include "libs/common/inluce/errors.h"

#include <sstream>

namespace vm_scheduler {

namespace {

ostream& what(ostream& out, const std::exception& ex)
{
    out << ex.what() << '\n';
    try {
        std::rethrow_if_nested(ex);
    } catch (const std::exception& nested) {
        out << nested;
    }
    return out;
}

} // anonymous namespace

const char* what(const std::exception_ptr& exPtr)
{
    try {
        std::rethrow_exception(exPtr);
    } catch (const std::exception& ex) {
        std::stringstream s;
        return what(s, ex).str();
    }
}

} // namespace vm_scheduler
