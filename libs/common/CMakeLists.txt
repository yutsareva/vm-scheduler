get_filename_component(LIB_INCLUDE_PATH "../../.." ABSOLUTE)
get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB LIB_HEADERS ${LIB_PATH}/*.h)

add_library(lib_common STATIC ${LIB_CXX_SOURCES} ${LIB_HEADERS})
target_include_directories(lib_common PUBLIC ${LIB_INCLUDE_PATH})
