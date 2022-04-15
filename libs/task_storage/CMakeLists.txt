get_filename_component(LIB_INCLUDE_PATH ".." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB_RECURSE LIB_CXX_HEADERS ${LIB_PATH}/*.h)

add_library(lib_task_storage STATIC ${LIB_CXX_SOURCES} ${LIB_CXX_HEADERS})
target_include_directories(lib_task_storage PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(lib_task_storage PUBLIC lib_common lib_state)
