#!/usr/bin/env bash
# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
# license. See LICENSE in the project root.
set -ueo pipefail

export PREFIX=/usr/local/

echo "Prefix set to $PREFIX"

export CMAKE_PREFIX_PATH=$PREFIX

echo "Installing dependencies"

pushd third_party/qnlp-toolkit
	rm -rf build
	bash install.sh $PREFIX
popd
 
for dep in pistache json
do
pushd third_party/$dep
	rm -rf build
	mkdir -p build
	pushd build
		cmake .. -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DCMAKE_BUILD_TYPE=Release
		make -j && make install
	popd
popd
done


echo "Installing text-nlu"
mkdir -p $PREFIX
rm -rf build
mkdir -p build
pushd build
	cmake -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DCMAKE_BUILD_TYPE=Release Protobuf_PROTOC_EXECUTABLE=/usr/local/bin/protoc ..
	make -j && make install
popd
