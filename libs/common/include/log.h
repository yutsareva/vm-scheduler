#pragma once

#include <libs/common/include/time.h>

#include <iostream>

#define INFO() std::cerr << std::endl << "[" << vm_scheduler::getCurrentTime() << "] "
#define DEBUG() std::cerr << std::endl << "[" << vm_scheduler::getCurrentTime() << "] "
#define ERROR() std::cerr << std::endl << "[" << vm_scheduler::getCurrentTime() << "] "
