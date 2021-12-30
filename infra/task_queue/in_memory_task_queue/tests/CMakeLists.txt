get_filename_component(LIB_PATH "." ABSOLUTE)

file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_PATH}/*.cpp)
file(GLOB_RECURSE LIB_HEADERS ${LIB_PATH}/*.h)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

add_executable(test_in_memory_task_queue ${LIB_CXX_SOURCES} ${LIB_HEADERS})


target_link_libraries(test_in_memory_task_queue ${Boost_LIBRARIES})
target_link_libraries(test_in_memory_task_queue in_memory_task_queue)

#message(STATUS "---------------------")
#message(STATUS "Boost_FOUND: ${Boost_FOUND}")
#message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
#message(STATUS "Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")
#message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
#message(STATUS "---------------------")
