#cmake_minimum_required(VERSION 3.22)
#project(vm_scheduler_lib_task_registry)
#set(CMAKE_CXX_STANDARD 20)

get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/impl/*.cpp)

add_library(lib_task_registry STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_task_registry PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(lib_task_registry PRIVATE proto_lib common_lib lib_server)
