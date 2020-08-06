
// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#ifndef __NLU_H
#define __NLU_H

#include <iostream>
#include <fstream>
#include <fcntl.h>

#include "grpcpp/create_channel.h"

#include <grpcpp/security/credentials.h>
#include <google/protobuf/map.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "tensorflow_serving/apis/model_service.grpc.pb.h"

#include "tokenizer.h"
#include "utils.h"


#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;
using tensorflow::serving::PredictionService;
using tensorflow::serving::ModelService;

typedef google::protobuf::Map<std::string, tensorflow::TensorProto> OutMap;

using namespace std;

// Flattens a 2D std::vector to a 1D std::vector.
template <typename T>
std::vector<T> FlattenVector(const std::vector<std::vector<T> >& vals) {
  std::vector<T> flat_vals;
  flat_vals.reserve(vals.size() * vals.front().size());
  for (const auto& v : vals) {
    flat_vals.insert(flat_vals.end(), v.cbegin(), v.cend());
  }
  return flat_vals;
}
// Flattens a 3D std::vector to a 1D std::vector.
template <typename T>
std::vector<T> FlattenVector(const std::vector<std::vector<std::vector<T> > >& vals) {
  std::vector<T> flat_vals;
  size_t final_size = vals.size() * vals.front().size() * vals.front().front().size();
//   flat_vals.reserve(vals.size() * vals.front().size() * vals.front().front().size() );
  for (const auto& values : vals) 
  {
      for (const auto& v : values) 
      {
          flat_vals.insert(flat_vals.end(), v.cbegin(), v.cend());
      }
  }
  return flat_vals;
}

class nlu
{
    public:
        nlu(int debug_mode, std::string model_config_path, std::string tfserving_host);
        ~nlu(){delete(_tokenizer);};
        std::vector<std::string> getDomains();
        bool getLocal();
        void setDebugMode(int debug_mode);
        std::vector <std::string> tokenize(std::string &input, std::string &lang, bool lowercase);
        std::vector <std::string> tokenize(std::string &input, std::string &lang);
        std::string tokenize_str(std::string &input, std::string &lang, bool lowercase);
        std::string detokenize_str(std::string &input, std::string &lang, bool lowercase);
        std::string tokenize_str(std::string &input, std::string &lang);
        std::string detokenize_str(std::string &input, std::string &lang);
        Status NLUBatch(
          std::vector<std::vector<std::string> >& batch_tokens,
          std::vector<std::vector<std::string> >& output_batch_tokens,
          std::string domain);
        Status NLUDecode(
          std::vector<std::vector<std::string> >& batch_tokens,
          std::vector<std::vector<std::string> >& output_batch_tokens,
          std::string domain);
        bool CheckModelsStatus();
    private:
      static map<tensorflow::serving::ModelVersionStatus_State, std::string> mapState;

      unique_ptr<PredictionService::Stub> _stub;
      shared_ptr<Channel> _channel;

      bool _local;
      tokenizer * _tokenizer;
      int _debug_mode;
      std::string _model_config_path;

      std::vector<int> PadBatch(
        std::vector<std::vector<std::string> >& batch_tokens);
      int getMaxLengthWord(std::vector<std::vector<std::string>>& batch_tokens);
      void getBatchCharsListFromBatchTokens(
        std::vector<std::vector<std::vector<std::string>>>& chars_list_batch,
        std::vector<std::vector<std::string>>& batch_tokens, 
        int max_length_word);
};


#endif // __NLU_H
