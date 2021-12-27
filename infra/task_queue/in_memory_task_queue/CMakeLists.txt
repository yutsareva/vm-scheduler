#project(in_memory_task_queue)
#cmake_minimum_required(VERSION 3.20)

get_filename_component(LIB_INCLUDE_PATH "../../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.hpp)

add_library(in_memory_task_queue STATIC ${LIB_CXX_SOURCES} ${LIB_ASM_SOURCES} ${LIB_HEADERS})
target_include_directories(in_memory_task_queue PUBLIC ${LIB_INCLUDE_PATH})

add_subdirectory(tests)
