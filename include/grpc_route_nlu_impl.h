// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#ifndef __GRPC_ROUTE_CLASSIFY_IMPL_H
#define __GRPC_ROUTE_CLASSIFY_IMPL_H

#include <grpcpp/grpcpp.h>
#include <grpc/grpc.h>
#include "grpc_nlu.grpc.pb.h"
#include "grpc_nlu.pb.h"

#include "tokenizer.h"
#include "nlu.h"
#include "abstract_server.h"
#include "utils.h"

class GrpcRouteNLUImpl : public RouteNLU::Service {
public:
    GrpcRouteNLUImpl(shared_ptr<nlu> nlu_ptr, int debug_mode);
    ~GrpcRouteNLUImpl() {};
private:
    grpc::Status GetDomains(grpc::ServerContext* context,
                            const Empty* request,
                            Domains* response) override;
    grpc::Status GetNLU(grpc::ServerContext* context,
                            const TextToParse* request,
                            TextParsed* response) override;
    grpc::Status StreamNLU(grpc::ServerContext* context,
                                grpc::ServerReaderWriter< TextParsed, TextToParse>* stream) override;

    void PrepareOutput(const TextToParse* request, TextParsed* response);

    void SetOutput(TextParsed* response, std::vector<std::vector<std::string>>& tokenized_batch, std::vector<std::vector<std::string>>& output_batch_tokens);

    shared_ptr<nlu> _nlu;
    int _debug_mode;
};

#endif // __GRPC_ROUTE_CLASSIFY_IMPL_H