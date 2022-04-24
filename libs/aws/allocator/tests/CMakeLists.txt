include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

enable_testing()

add_executable(
        test_aws_allocator
        aws_allocator.cpp
)
target_link_libraries(
        test_aws_allocator
        PUBLIC
        gtest_main
        gmock_main
        aws_allocator
        lib_state
)

include(GoogleTest)
gtest_discover_tests(test_aws_allocator)
