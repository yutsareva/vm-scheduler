cmake_minimum_required(VERSION 3.22)
project(vm_scheduler_lib_allocator)
set(CMAKE_CXX_STANDARD 20)

get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.h)

add_library(lib_allocator STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_allocator PUBLIC ${LIB_INCLUDE_PATH})
