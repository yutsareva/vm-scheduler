include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

enable_testing()
get_filename_component(TESTS_PATH "." ABSOLUTE)
file(GLOB LIB_CXX_SOURCES ${TESTS_PATH}/*.cpp)

add_executable(
        test_bin
        ${LIB_CXX_SOURCES}
)

add_dependencies(test_bin proto_lib)
target_link_libraries(
        test_bin
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
gtest_discover_tests(test_bin)
