get_filename_component(LIB_INCLUDE_PATH "../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/impl/*.cpp)

#find_package(zkpp REQUIRED)

add_library(lib_zk STATIC ${LIB_CXX_SOURCES})
target_include_directories(lib_zk PUBLIC ${LIB_INCLUDE_PATH})
#target_link_libraries(lib_zk PUBLIC lib_common lib_state)
#target_link_libraries (lib_zk INTERFACE zkpp)
find_package(zkpp CONFIG REQUIRED)
target_link_libraries(lib_zk PUBLIC zkpp)
