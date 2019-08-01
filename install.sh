#!/usr/bin/env bash
# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
# license. See LICENSE in the project root.


export PREFIX=/usr/local/

echo "Prefix set to $PREFIX"

export CMAKE_PREFIX_PATH=$PREFIX

git submodule update --init --recursive

echo "Installing dependencies"

pushd vendor/qnlp-toolkit
	rm -rf build
	git pull  --recurse-submodules 
	bash install.sh $PREFIX
popd
 
for dep in pistache json grpc
do
pushd vendor/$dep
	rm -rf build
	mkdir -p build
	pushd build
		cmake .. -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DCMAKE_BUILD_TYPE=Release
		make -j && make install
	popd
popd
done

#pushd vendor/grpc
#	./configure
#	make -j
#	make install
#popd

exit

pushd vendor/tensorflow
echo "tf_cc_shared_object(  \
    name = \"libtensorflow_qnlp.so\",  \
    linkopts = select({  \
        \"//tensorflow:darwin\": [  \
            \"-Wl,-exported_symbols_list\",   \
            \"//tensorflow:tf_exported_symbols.lds\",  \
        ],  \
        \"//tensorflow:windows\": [],  \
        \"//tensorflow:windows_msvc\": [],  \
        \"//conditions:default\": [  \
            \"-z defs\",  \
            \"-s\",  \
            \"-Wl,--version-script\",  \
            \"//tensorflow:tf_version_script.lds\",  \
        ],  \
    }),  \
    deps = [  \
        \"//tensorflow:tf_exported_symbols.lds\",  \
        \"//tensorflow:tf_version_script.lds\",  \
        \"//tensorflow/c:c_api\",  \
        \"//tensorflow/c/eager:c_api\",  \
        \"//tensorflow/cc:cc_ops\",  \
        \"//tensorflow/cc:client_session\",  \
        \"//tensorflow/cc:scope\",  \
        \"//tensorflow/core:tensorflow\",  \
        \"//tensorflow/contrib/seq2seq:beam_search_ops_kernels\",  \
        \"//tensorflow/contrib/seq2seq:beam_search_ops_op_lib\",  \
    ],  \
)  \
" >> tensorflow/BUILD
./configure
bazel build  --config=opt //tensorflow:libtensorflow_qnlp.so
popd

exit

echo "Installing text-nlu"
mkdir -p $PREFIX
rm -rf build
mkdir -p build
pushd build
	cmake .. -DCMAKE_INSTALL_PREFIX="${PREFIX}" -DCMAKE_BUILD_TYPE=Release 
	make -j && make install
popd
