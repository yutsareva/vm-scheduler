#cmake_minimum_required(VERSION 3.22)
#project(vm_scheduler_proto)
#set(CMAKE_CXX_STANDARD 20)
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Protobuf CONFIG REQUIRED)

find_package(gRPC CONFIG REQUIRED)
find_package(Threads REQUIRED)
#
# Protobuf/Grpc source files
#
set(PROTO_FILES
        services/agent_api.proto
        services/public_api.proto

        structures/agent.proto
        structures/job.proto
        structures/task.proto
        structures/task_addition_result.proto
        structures/task_execution_result.proto
        structures/task_state.proto
)

#
# Add Library target with protobuf sources
#
add_library(proto_lib ${PROTO_FILES})
target_link_libraries(proto_lib PUBLIC pthread)



#target_link_libraries(proto_lib PRIVATE ${CMAKE_THREAD_LIBS_INIT} Threads::Threads)
target_link_libraries(proto_lib
        PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)

target_include_directories(proto_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# Compile protobuf and grpc files in proto target to cpp
#
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET proto_lib LANGUAGE cpp)
protobuf_generate(TARGET proto_lib LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
