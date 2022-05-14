get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/impl/*.cpp)

add_library(lib_scheduler STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_scheduler PUBLIC ${LIB_INCLUDE_PATH})
add_dependencies(lib_scheduler lib_zk)
target_link_libraries(lib_scheduler lib_zk lib_common)
