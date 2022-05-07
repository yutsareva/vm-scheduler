#pragma once

#include <libs/common/include/time.h>

#include <iostream>

#define INFO() std::cerr << "\n[" << vm_scheduler::getCurrentTime() << "] "
#define DEBUG() std::cerr << "\n[" << vm_scheduler::getCurrentTime() << "] "
#define ERROR() std::cerr << "\n[" << vm_scheduler::getCurrentTime() << "] "
