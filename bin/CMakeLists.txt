get_filename_component(LIB_INCLUDE_PATH ".." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/main.cpp)

add_executable(server ${LIB_CXX_SOURCES} ${LIB_CXX_HEADERS})
target_include_directories(server PUBLIC ${LIB_INCLUDE_PATH})
add_dependencies(server proto_lib)
target_link_libraries(server PUBLIC proto_lib lib_task_registry db lib_scheduler aws_allocator lib_postgres)
