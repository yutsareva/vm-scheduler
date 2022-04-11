#pragma once

#include <libs/common/include/errors.h>


namespace vm_scheduler {

class JobNotFoundException : public RuntimeException {
    using RuntimeException::RuntimeException;
};
class JobCancelledException : public RuntimeException {
    using RuntimeException::RuntimeException;
};

} // namespace vm_scheduler
