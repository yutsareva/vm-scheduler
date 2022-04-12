cmake_minimum_required(VERSION 3.22)
project(vm_scheduler_postgres_lib)
set(CMAKE_CXX_STANDARD 20)

get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)

add_library(postgres_lib STATIC ${LIB_CXX_SOURCES})
include_directories(${LIB_INCLUDE_PATH})
#target_include_directories(postres_lib PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(postgres_lib)
#target_link_libraries(postgres_lib -lpqxx -lpq)
