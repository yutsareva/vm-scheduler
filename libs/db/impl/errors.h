#pragma once

#include <libs/common/include/errors.h>

#include <string>


namespace vm_scheduler {

class LockUnavailable : public RuntimeException {
    using RuntimeException::RuntimeException;
};
class PgException : public RuntimeException {
    using RuntimeException::RuntimeException;
};
class InconsistentPgResultException : public RuntimeException {
    using RuntimeException::RuntimeException;
};
class SchedulingCancelled : public RuntimeException {
    using RuntimeException::RuntimeException;
};
class OptimisticLockingFailure : public RuntimeException {
    using RuntimeException::RuntimeException;
};

} // namespace vm_scheduler
