get_filename_component(LIB_INCLUDE_PATH ".." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)

add_executable(bin ${LIB_CXX_SOURCES} ${LIB_CXX_HEADERS})
target_include_directories(bin PUBLIC ${LIB_INCLUDE_PATH})
add_dependencies(bin proto_lib)
target_link_libraries(bin PUBLIC proto_lib lib_task_registry db lib_scheduler aws_allocator lib_postgres)
