cmake_minimum_required(VERSION 3.22)
project(vm_scheduler_proto)
set(CMAKE_CXX_STANDARD 20)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

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
cmake_minimum_required(VERSION 3.22)
project(proto_lib)

find_package(Protobuf REQUIRED)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES} PLUGIN protoc-gen-grpc=${grpc_cpp_plugin_location})
add_library(proto_lib STATIC ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(proto_lib PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(proto_lib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(proto_lib ${Protobuf_LIBRARIES})
