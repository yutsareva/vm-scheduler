get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.h)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_executable(test_aws_allocator ${LIB_CXX_SOURCES} ${LIB_HEADERS})

target_link_libraries(test_aws_allocator ${Boost_LIBRARIES})
target_link_libraries(test_aws_allocator aws_allocator)
