get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.hpp)

add_library(aws_allocator STATIC ${LIB_CXX_SOURCES} ${LIB_ASM_SOURCES} ${LIB_HEADERS})
target_include_directories(aws_allocator PUBLIC ${LIB_INCLUDE_PATH})
