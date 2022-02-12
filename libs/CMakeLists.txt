project(vm_scheduler_lib)

#set(CMAKE_CXX_STANDARD 20)

add_subdirectory(allocator)
#add_subdirectory(blob_storage)
add_subdirectory(common)
add_subdirectory(scheduler)
add_subdirectory(state)
add_subdirectory(task_registry)
add_subdirectory(task_storage)
