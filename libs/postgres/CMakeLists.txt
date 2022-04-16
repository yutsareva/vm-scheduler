get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)

add_library(lib_postgres STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_postgres PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(lib_postgres -lpqxx -lpq)
