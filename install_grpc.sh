#!/usr/bin/env bash
# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
# license. See LICENSE in the project root.
set -ueo pipefail

export PREFIX=/usr/local/

echo "Prefix set to $PREFIX"

export CMAKE_PREFIX_PATH=$PREFIX


pushd vendor/grpc
    # Based on https://github.com/grpc/grpc/blob/master/test/distrib/cpp/run_distrib_test_cmake.sh

    # Install c-ares
    pushd third_party/cares/cares
        mkdir -p cmake/build
        pushd cmake/build
            cmake -DCMAKE_BUILD_TYPE=Release ../..
            make -j4 install
        popd
    popd
    # rm -rf third_party/cares/cares  # wipe out to prevent influencing the grpc build

    # Install zlib
    pushd third_party/zlib
        mkdir -p cmake/build
        pushd cmake/build
            cmake -DCMAKE_BUILD_TYPE=Release ../..
            make -j4 install
        popd
    popd
    # rm -rf third_party/zlib  # wipe out to prevent influencing the grpc build

    # Install protobuf
    pushd third_party/protobuf
        mkdir -p cmake/build
        pushd cmake/build
            cmake -Dprotobuf_BUILD_TESTS=OFF -DCMAKE_BUILD_TYPE=Release ..
            make -j4 install
        popd
    popd
    # rm -rf third_party/protobuf  # wipe out to prevent influencing the grpc build

    rm -rf cmake/build
    mkdir -p cmake/build
    pushd cmake/build
        cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF -DgRPC_PROTOBUF_PROVIDER=package -DgRPC_ZLIB_PROVIDER=package -DgRPC_CARES_PROVIDER=package -DgRPC_SSL_PROVIDER=package -DCMAKE_BUILD_TYPE=Release --DCMAKE_INSTALL_PREFIX="${PREFIX}" ../..
        # See https://github.com/grpc/grpc/issues/13841
        make -j 4 && make install
    popd
popd
