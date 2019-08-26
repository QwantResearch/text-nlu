# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
PROJECT(text-nlu)

cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpthread")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)


set(TF_DIR vendor/tensorflow/)
set(TF_SERVING_DIR vendor/tfserving/)
include_directories(BEFORE ${TF_DIR} ${TF_DIR}/bazel-genfiles ${TF_DIR}/bazel-tensorflow/external/eigen_archive ${TF_DIR}/bazel-tensorflow/external/protobuf_archive/src ${TF_DIR}/bazel-tensorflow/external/nsync/public/ ${TF_SERVING_DIR} ${TF_SERVING_DIR}/bazel-genfiles ${TF_SERVING_DIR}bazel-tfserving/external/grpc/include/)
link_directories(BEFORE ${TF_DIR}/bazel-bin/tensorflow ${TF_SERVING_DIR}/bazel-bin/tensorflow_serving/model_servers/ ${TF_SERVING_DIR}/bazel-bin/tensorflow_serving/apis/ ${TF_SERVING_DIR}/bazel-bin/tensorflow_serving/util/ ${TF_SERVING_DIR}/bazel-serving/bazel-out/k8-opt/bin/tensorflow_serving/core/)


include_directories(${PROJECT_SOURCE_DIR}/include/ /usr/local/include/ /usr/)

link_directories(/usr/local/lib/ /usr/lib/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -lpthread")

set(CLASS_EXECUTABLE_API text-nlu)

set(CLASS_SOURCE_API ${PROJECT_SOURCE_DIR}/src/text-nlu.cpp
                     ${PROJECT_SOURCE_DIR}/src/nlu.cpp
                     ${PROJECT_SOURCE_DIR}/src/rest_server.cpp
                     ${PROJECT_SOURCE_DIR}/src/tokenizer.cpp
                     ${PROJECT_SOURCE_DIR}/src/utils.cpp)

add_executable(${CLASS_EXECUTABLE_API} ${CLASS_SOURCE_API})

set(LIBS
    fasttext
    qnlp
    pistache
    pthread
    yaml-cpp

    tensorflow_framework 
    tensorflow_qnlp  
    predict_proto 
    model_proto 
    optional 
    grpc++ 
    grpc
    prediction_service_proto 
    classification_proto 
    get_model_metadata_proto 
    regression_proto 
    input_proto 
    inference_proto
)

target_link_libraries(${CLASS_EXECUTABLE_API} ${LIBS})

install(TARGETS ${CLASS_EXECUTABLE_API} DESTINATION bin)
