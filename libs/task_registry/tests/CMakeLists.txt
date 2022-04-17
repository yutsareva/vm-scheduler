include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        test_task_registry
        task_registry.cpp
)
add_dependencies(test_task_registry proto_lib)
target_link_libraries(
        test_task_registry
        gtest_main
        gmock_main
        proto_lib
        lib_server
        lib_task_registry
)

include(GoogleTest)
gtest_discover_tests(test_task_registry)
