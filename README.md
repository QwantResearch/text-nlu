Text NLU
=========

A new C++ API for NLU at Qwant Research.
The API is based on Tensorflow.

## Installation
```
git clone --recursive https://github.com/QwantResearch/text-nlu.git 
cd text-nlu
docker build -t text-nlu:latest .
``` 

## Start using Docker Compose
This is the easiest way to start text-nlu service.
Docker-compose starts both the TFserving and the text-nlu:
```
docker-compose up
```
You can change environment variables in the docker-compose.yml.

## Starting text-nlu locally
If you want to start text-nlu locally, make sure you have a tensorflow serving server running.
Then launch text-nlu using:
```
./text-nlu [--threads <nthreads>] [--port <port>] [--grpc] [--debug] 
           --model_config_path <filename> --tfserving_host <address:port>

        --threads (-t)           number of threads (default 1)
        --port (-p)              port to use (default 9009)
        --grpc (-g)              use grpc service instead of rest
        --debug (-d)             debug mode (default false)
        --model_config_path (-c) model_config_path file in which API configuration is set (needed)
        --tfserving_host (-s)    TFServing host (needed)
        --help (-h)              Show this message
```
You can also set environment variables instead of args, as in docker-compose.yml file.

## Licencing

Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

Contact:
 - e[dot]maudet[at]qwantresearch[dot]com
 - p[dot]jackman[at]qwantresearch[dot]com
 - n[dot]martin[at]qwantresearch[dot]com
 - christophe[dot]servan[at]qwantresearch[dot]com
