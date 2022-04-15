cmake_minimum_required(VERSION 3.22)
#set(CMAKE_TOOLCHAIN_FILE "/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(vm_scheduler)

set(CMAKE_CXX_STANDARD 20)
#add_compile_options(-stdlib=libc++)

add_subdirectory(libs)
add_subdirectory(proto)
