#pragma once

#include <exception>
#include <string>


namespace vm_scheduler {

const char* what(const std::exception_ptr& exPtr);

} // namespace vm_scheduler
