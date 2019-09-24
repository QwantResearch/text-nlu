// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#ifndef __REST_SERVER_H
#define __REST_SERVER_H

#include <algorithm>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <pistache/client.h>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <sstream>
#include <time.h>
#include "yaml-cpp/yaml.h"
#include <grpcpp/grpcpp.h>

#include "abstract_server.h"

using namespace std;
using namespace nlohmann;
using namespace Pistache;

using grpc::Status;

class rest_server : public AbstractServer {

public:
  using AbstractServer::AbstractServer;
  ~rest_server(){httpEndpoint->shutdown();};

  void init(size_t thr = 2) override;
  void start() override;
  void shutdown() override;

private:
  std::shared_ptr<Http::Endpoint> httpEndpoint;
  Rest::Router router;

  void setupRoutes();

  void doNLUGet(const Rest::Request &request,
                           Http::ResponseWriter response);

  void doNLUPost(const Rest::Request &request,
                            Http::ResponseWriter response);

  void doNLUBatchPost(const Rest::Request &request,
                                 Http::ResponseWriter response);

  void fetchParamWithDefault(const json& j, 
                              string& domain, 
                              string& lang, 
                              int& count,
                              float& threshold,
                              bool& debugmode);
  Status askNLU(std::string &text, std::string &tokenized, json &output, string &domain, string &lang, bool debugmode);
  Status askNLU(vector<vector<string> > &input, json &output, string &domain, string &lang, bool debugmode);

  void writeLog(string text_to_log) {}

  void doAuth(const Rest::Request &request, Http::ResponseWriter response);
};

#endif // __REST_SERVER_H
