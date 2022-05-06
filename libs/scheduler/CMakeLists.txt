get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/impl*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.h)

add_library(lib_scheduler STATIC ${LIB_CXX_SOURCES} ${LIB_HEADERS})
target_include_directories(lib_scheduler PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(lib_scheduler lib_common)
