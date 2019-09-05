# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
PROJECT(text-nlu)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

# Protobuf compiler dependency.
include(CompileProto.cmake)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP_UNSECURE gRPC::grpc++_unsecure)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Protobuf sources of the TensorFlow Serving to be compiled without a gRPC plugin.
file(GLOB_RECURSE TF_PROTOS protos/*.proto)

# Compiling CPP sources from proto files.
compile_proto(0 "${CMAKE_SOURCE_DIR}/protos" "${CMAKE_SOURCE_DIR}/compiled" PB_SOURCES PB_HEADERS ${TF_PROTOS})

# Compiling CPP sources with gRPC plugin.
compile_proto(1 "${CMAKE_SOURCE_DIR}/protos" "${CMAKE_SOURCE_DIR}/compiled" PB_GRPC_SOURCES PB_GRPC_HEADERS
        protos/tensorflow_serving/apis/prediction_service.proto
        protos/tensorflow_serving/config/model_server_config.proto
        protos/grpc_nlu.proto
)

# Including compiled files.
include_directories(compiled)


include_directories(${PROJECT_SOURCE_DIR}/include/ /usr/local/include/ /usr/)

link_directories(/usr/local/lib/ /usr/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -lpthread")

set(CLASS_EXECUTABLE_API text-nlu)

set(CLASS_SOURCE_API ${PROJECT_SOURCE_DIR}/src/text-nlu.cpp
                     ${PROJECT_SOURCE_DIR}/src/nlu.cpp
                     ${PROJECT_SOURCE_DIR}/src/rest_server.cpp
                     ${PROJECT_SOURCE_DIR}/src/tokenizer.cpp
                     ${PROJECT_SOURCE_DIR}/src/utils.cpp
                     ${PROJECT_SOURCE_DIR}/src/abstract_server.cpp
                     ${PROJECT_SOURCE_DIR}/src/grpc_route_nlu_impl.cpp
                     ${PROJECT_SOURCE_DIR}/src/grpc_server.cpp

                     ${PB_GRPC_HEADERS}
                     ${PB_GRPC_SOURCES}
                     ${PB_HEADERS}
                     ${PB_SOURCES}
)




add_executable(${CLASS_EXECUTABLE_API} ${CLASS_SOURCE_API})

set(LIBS
    ${_GRPC_GRPCPP_UNSECURE}
    ${_PROTOBUF_LIBPROTOBUF}
    fasttext
    qnlp
    pistache
    pthread
    yaml-cpp
)

target_link_libraries(${CLASS_EXECUTABLE_API} ${LIBS})

install(TARGETS ${CLASS_EXECUTABLE_API} DESTINATION bin)
