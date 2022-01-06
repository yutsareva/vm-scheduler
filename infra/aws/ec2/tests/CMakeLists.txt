get_filename_component(LIB_PATH "." ABSOLUTE)
get_filename_component(LIB_INCLUDE_PATH "../../../.." ABSOLUTE)

file(GLOB LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB LIB_HEADERS ${LIB_PATH}/*.h)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_executable(test_aws_allocator ${LIB_CXX_SOURCES} ${LIB_HEADERS})

target_link_libraries(test_aws_allocator ${Boost_LIBRARIES} aws_allocator)
target_include_directories(test_aws_allocator PUBLIC ${LIB_INCLUDE_PATH})
