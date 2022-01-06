get_filename_component(LIB_INCLUDE_PATH "../../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB LIB_HEADERS ${LIB_PATH}/*.h)

find_package(AWSSDK CONFIG COMPONENTS ec2 core dynamodb kinesis s3 REQUIRED)
find_package(ZLIB)

add_library(aws_allocator STATIC ${LIB_CXX_SOURCES} ${LIB_HEADERS})
target_include_directories(aws_allocator PUBLIC ${LIB_INCLUDE_PATH})

target_include_directories(aws_allocator PUBLIC ${AWSSDK_INCLUDE_DIRS})
target_link_libraries(aws_allocator PUBLIC ${AWSSDK_LIBRARIES})
