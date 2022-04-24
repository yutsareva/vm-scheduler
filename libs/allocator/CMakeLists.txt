get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/impl/*.cpp)

add_library(lib_allocator STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_allocator PUBLIC ${LIB_INCLUDE_PATH})
