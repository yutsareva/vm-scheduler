get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/impl/*.cpp)

add_library(lib_failure_detector STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_failure_detector PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(lib_failure_detector PRIVATE lib_common lib_state lib_allocator lib_task_storage)
