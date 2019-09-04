// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "grpc_route_nlu_impl.h"

grpc::Status GrpcRouteNLUImpl::GetDomains(grpc::ServerContext* context,
                                               const Empty* request,
                                               Domains* response) {

  // for (auto& it: _nlu->getListClassifs()) {
  //   response->add_domains(it->getDomain());
  // }
  if (_debug_mode)
    cerr << "[DEBUG]: " << currentDateTime() << "\t" << "GetDomains" << endl;
  return grpc::Status::OK;
}

grpc::Status GrpcRouteNLUImpl::GetNLU(grpc::ServerContext* context,
                                               const TextToParse* request,
                                               TextParsed* response) {

  if (_debug_mode) {
    cerr << "[DEBUG]: " << currentDateTime() << "\t" << "GetNLU";
    cerr << "\t" << request->text() << "\t";
  }

  PrepareOutput(request, response);

  std::string text = response->text();
  std::string domain = response->domain();
  std::string lang = response->lang();
  std::string tokenized;

  std::string tokenized_text = _nlu->tokenize_str(text, lang);
  std::vector<std::string> tokenized_vec = _nlu->tokenize(text, lang);

  vector<vector<string> > tokenized_batched;
  tokenized_batched.push_back(tokenized_vec);
 
  std::vector<std::vector<std::string>> output_batch_tokens;
  _nlu->NLUBatch(tokenized_batched, output_batch_tokens, domain);
  response->set_tokenized(tokenized_text);

  SetOutput(response, tokenized_batched, output_batch_tokens);

  return grpc::Status::OK;
}

grpc::Status GrpcRouteNLUImpl::StreamNLU(grpc::ServerContext* context,
                                                   grpc::ServerReaderWriter< TextParsed,
                                                                            TextToParse>* stream) {
  TextToParse request;
  while (stream->Read(&request)) {

    if (_debug_mode) {
      cerr << "[DEBUG]: " << currentDateTime() << "\t" << "StreamNLU";
      cerr  << "\t" << request.text() << "\t";
    }

    TextParsed response;
    PrepareOutput(&request, &response);

    std::string text = response.text();
    std::string domain = response.domain();
    std::string lang = response.lang();
    std::string tokenized;


    std::string tokenized_text = _nlu->tokenize_str(text, lang);
    std::vector<std::string> tokenized_vec = _nlu->tokenize(text, lang);

    vector<vector<string> > tokenized_batched;
    tokenized_batched.push_back(tokenized_vec);
  
    std::vector<std::vector<std::string>> output_batch_tokens;
    _nlu->NLUBatch(tokenized_batched, output_batch_tokens, domain);
    response.set_tokenized(tokenized_text);

    SetOutput(&response, tokenized_batched, output_batch_tokens);

    stream->Write(response);
  }

  return grpc::Status::OK;
}

void GrpcRouteNLUImpl::PrepareOutput(const TextToParse* request, TextParsed* response) {
  std::string text = request->text();

  response->set_text(request->text());
  response->set_domain(request->domain());
  response->set_count(request->count());
  response->set_lang(request->lang());
}

void GrpcRouteNLUImpl::SetOutput(
  TextParsed* response, 
  std::vector<std::vector<std::string>>& tokenized_batch, 
  std::vector<std::vector<std::string>>& output_batch_tokens) {
  for (int i = 0; i < tokenized_batch.size(); i++) {
    std::vector<std::string> tokenized = tokenized_batch[i];
    Tag *tag;
    for (int j = 0; j < tokenized.size(); j++) {
      std::string current_word = tokenized[j];
      std::string bio = output_batch_tokens[i][j];

      if (j == 0 || bio.find("B-") == 0 || (bio.find("O") == 0 && tag->tag().find("O") != 0)) { 
        // either a "B-" or a "O" that doesn't follow another "O": we create a new tag
        tag = response->add_tag();
        tag->set_phrase(current_word);
        tag->set_tag(bio);
      } else { // we have a "O" that follows another "O" or a "I": we append text to phrase
        tag->set_phrase(tag->phrase() + " " + current_word);
      }
    }
  }
}

GrpcRouteNLUImpl::GrpcRouteNLUImpl(shared_ptr<nlu> nlu_ptr, int debug_mode) {
  _nlu = nlu_ptr;
  _debug_mode = debug_mode;
}