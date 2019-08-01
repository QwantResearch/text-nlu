#!/usr/bin/env bash
# Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
# license. See LICENSE in the project root.


pushd vendor/tensorflow
if [ ! -e tensorflow/BUILD.bck ]
then
	cp tensorflow/BUILD tensorflow/BUILD.bck -v 
else
	cp tensorflow/BUILD.bck tensorflow/BUILD -v 
fi


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
