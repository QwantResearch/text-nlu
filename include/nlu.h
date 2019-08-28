
// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#ifndef __NLU_H
#define __NLU_H

#include <iostream>
#include <fstream>

#include "grpcpp/create_channel.h"

#include <grpcpp/security/credentials.h>
#include <google/protobuf/map.h>

#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"

#include "tokenizer.h"


#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;
using tensorflow::serving::PredictionService;

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

// // Convenience functions to convert std::vectors to tensorflow::Tensors. 
// template <typename T>
// tensorflow::TensorProto AsTensor(const std::vector<T>& vals) {
//   tensorflow::TensorProto ret(std::DataTypeToEnum<T>::value,
//                         {static_cast<std::int64>(vals.size())});
//   std::copy_n(vals.data(), vals.size(), ret.flat<T>().data());
//   return ret;
// }

// template <typename T>
// tensorflow::TensorProto AsTensor(const std::vector<T>& vals,
//                             const tensorflow::TensorShape& shape) {
//   tensorflow::TensorProto ret;
//   ret.CopyFrom(AsTensor(vals), shape);
//   return ret;
// }

// template <typename T>
// tensorflow::TensorProto AsTensorProto(const std::vector<T>& vals) {
//   tensorflow::TensorProto ret(tensorflow::DataTypeToEnum<T>::value,
//                         {static_cast<tensorflow::int64>(vals.size())});
//   std::copy_n(vals.data(), vals.size(), ret.flat<T>().data());
//   return ret;
// }
// 
// template <typename T>
// tensorflow::TensorProto AsTensorProto(const std::vector<T>& vals,
//                             const tensorflow::TensorShape& shape) {
//   tensorflow::TensorProto ret;
//   ret.CopyFrom(AsTensor(vals), shape);
//   return ret;
// }

class nlu
{
    public:
        nlu();
        nlu(const std::string&  filename,std::string& model_domain_param,std::string& lang);
        ~nlu(){delete(_tokenizer);};
        std::string callPredict(std::string& model_domain_param);
        bool getLocal();
        void setDebugMode(int debug_mode);
        std::string getDomain() { return _domain; }
        std::string getLang() { return _lang; }
        std::vector <std::string> tokenize(std::string &input);
        std::string tokenize_str(std::string &input);
    private:
      unique_ptr<PredictionService::Stub> _stub;

      bool _local;
      string _domain;
      string _lang;
      tokenizer * _tokenizer;
      int _debug_mode;
 
};


#endif // __NLU_H
