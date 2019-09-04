// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#ifndef __ABSTRACT_SERVER_H
#define __ABSTRACT_SERVER_H

#include <iostream>
#include <algorithm>

#include "yaml-cpp/yaml.h"

#include "nlu.h"

using namespace std;

class AbstractServer {
public:
    AbstractServer(string &config_file, int debug_mode);
    virtual ~AbstractServer() {}
    virtual void init(size_t thr = 2) = 0;
    virtual void start() = 0;
    virtual void shutdown() = 0;

protected:
  int _debug_mode;
  int _num_port;
  int _nbr_threads;
  std::string _model_config_path;
  std::shared_ptr<nlu> _nlu;

  void ProcessConfigFile(std::string &config_file);
};

#endif // __ABSTRACT_SERVER_H