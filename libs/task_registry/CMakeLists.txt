get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/impl/*.cpp)

add_library(lib_task_registry STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_task_registry PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(
        lib_task_registry PRIVATE
    proto_lib lib_allocator lib_common lib_server lib_concurrency lib_scheduler lib_failure_detector)
