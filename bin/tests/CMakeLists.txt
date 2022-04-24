include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
        test_fd_with_pg
        failure_detector.cpp
)
add_dependencies(test_fd_with_pg proto_lib)
target_link_libraries(
        test_fd_with_pg
        gtest_main
        gmock_main
        proto_lib
        db
        lib_allocator
        lib_server
        lib_task_registry
        lib_task_storage
        lib_postgres
)

include(GoogleTest)
gtest_discover_tests(test_fd_with_pg)
