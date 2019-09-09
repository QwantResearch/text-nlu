Text NLU
=========

A new C++ API for NLU at Qwant Research.
The API is based on [`fasttext`](https://fasttext.cc/) and Tensorflow.

## Installation
```
git clone --recursive https://github.com/QwantResearch/text-nlu.git 
cd text-nlu
docker build -t text-nlu:latest .
``` 

## Start a container network
```
docker network create text-nlu-net
```

## Launch the TFserving
```
docker run -t --rm -p 8500:8500 -v "$(pwd)/models/:/models/" --name tensorflow_serving --network text-nlu-net tensorflow/serving:latest  --model_config_file=/models/models_config.yaml
```

## Launch the API
```
docker run -p 9009:9009 --name text_nlu --network text-nlu-net text-nlu:latest --model-config <filename> [--port <port>] [--threads <nthreads>] [--debug]

--model-config (-c)      config file in which all models are described (REQUIRED)
--threads (-t)           number of threads (default 1)
--debug (-d)             debug mode (default false)
--help (-h)              show this message
```

## Licencing

Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

Contact:
 - e[dot]maudet[at]qwantresearch[dot]com
 - p[dot]jackman[at]qwantresearch[dot]com
 - n[dot]martin[at]qwantresearch[dot]com
 - christophe[dot]servan[at]qwantresearch[dot]com
