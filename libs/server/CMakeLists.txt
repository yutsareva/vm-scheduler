find_package(RapidJSON CONFIG REQUIRED)

get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)

add_library(lib_server STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_server PUBLIC ${LIB_INCLUDE_PATH})
target_link_libraries(lib_server PRIVATE proto_lib lib_common lib_state rapidjson)
