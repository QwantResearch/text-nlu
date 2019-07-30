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

## Launch the API
```
./text-nlu --model-config <filename> [--port <port>] [--threads <nthreads>] [--debug]

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
