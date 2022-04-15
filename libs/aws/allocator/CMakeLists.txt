cmake_minimum_required(VERSION 3.22)
project(vm_scheduler_aws_allocator)
set(CMAKE_CXX_STANDARD 20)

get_filename_component(LIB_INCLUDE_PATH "../../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)

find_package(AWSSDK CONFIG COMPONENTS ec2 core dynamodb kinesis s3 REQUIRED)
find_package(ZLIB)

add_library(aws_allocator STATIC ${LIB_CXX_SOURCES})
target_include_directories(aws_allocator PUBLIC ${LIB_INCLUDE_PATH})

target_include_directories(aws_allocator PUBLIC ${AWSSDK_INCLUDE_DIRS})
target_link_libraries(aws_allocator PUBLIC ${AWSSDK_LIBRARIES} lib_common)
