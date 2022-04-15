#pragma once

#include <libs/common/include/errors.h>

namespace vm_scheduler {

class InputValidationException : public RuntimeException {
    using RuntimeException::RuntimeException;
};

} // namespace vm_scheduler
