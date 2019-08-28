// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.
#include "nlu.h"

using namespace std;

void Split_str(const std::string& line, std::vector<std::string>& pieces, const std::string del) {
  size_t begin = 0;
  size_t pos = 0;
  std::string token;
  while ((pos = line.find(del, begin)) != std::string::npos) {
    if (pos > begin) {
      token = line.substr(begin, pos - begin);
      if(token.size() > 0)
        pieces.push_back(token);
    }
    begin = pos + del.size();
  }
  if (pos > begin) {
    token = line.substr(begin, pos - begin);
  }
  if(token.size() > 0)
    pieces.push_back(token);
}

bool nlu::getLocal()
{
    return _local;
}


nlu::nlu(const std::string& export_dir,std::string& model_domain_param,std::string& lang)
{
    std::cout << "NLU Init" << std::endl;
    std::string channel("kind_nightingale:8500");
    _stub = PredictionService::NewStub(CreateChannel(channel, grpc::InsecureChannelCredentials()));
    // _stub = PredictionService::PredictionService::NewStub(CreateChannel(channel, grpc::InsecureChannelCredentials()));
    std::cout << "NLU Init done" << std::endl;
    _local=true;
    _debug_mode=0;
}

nlu::nlu()
{
    cerr << "Warning: NLU object is empty" <<endl;
    _local=true;
    _debug_mode=0;
}

// Process a batch of tokenizes sentences.
std::string nlu::callPredict(std::string& model_name) {

  // Data we are sending to the server.
  PredictRequest request;
  request.mutable_model_spec()->set_name(model_name);

  // Container for the data we expect from the server.
  PredictResponse response;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  google::protobuf::Map<std::string, tensorflow::TensorProto>& inputs =
    *request.mutable_inputs();

  tensorflow::TensorProto proto;
  proto.set_dtype(tensorflow::DataType::DT_FLOAT);
  // proto.add_float_val(measurement);

  proto.mutable_tensor_shape()->add_dim()->set_size(5);
  proto.mutable_tensor_shape()->add_dim()->set_size(8);
  proto.mutable_tensor_shape()->add_dim()->set_size(105);

  inputs["inputs"] = proto;

  // The actual RPC.
  Status status = _stub->Predict(&context, request, &response);

  // Act upon its status.
  if (status.ok()) {
    std::cout << "call predict ok" << std::endl;
    std::cout << "outputs size is " << response.outputs_size() << std::endl;

    OutMap& map_outputs = *response.mutable_outputs();
    OutMap::iterator iter;
    int output_index = 0;

    for (iter = map_outputs.begin(); iter != map_outputs.end(); ++iter) {
      tensorflow::TensorProto& result_tensor_proto = iter->second;
      std::string section = iter->first;
      std::cout << std::endl << section << ":" << std::endl;

      if ("classes" == section) {
        int titer;
        for (titer = 0; titer != result_tensor_proto.int64_val_size(); ++titer) {
          std::cout << result_tensor_proto.int64_val(titer) << ", ";
        }
      } else if ("scores" == section) {
        int titer;
        for (titer = 0; titer != result_tensor_proto.float_val_size(); ++titer) {
          std::cout << result_tensor_proto.float_val(titer) << ", ";
        }
      }
      std::cout << std::endl;
      ++output_index;
    }
    return "Done.";
  } else {
    std::cout << "gRPC call return code: " << status.error_code() << ": "
              << status.error_message() << std::endl;
    return "RPC failed";
  }
  return "Yeah";
}

void nlu::setDebugMode(int debug_mode)
{
    _debug_mode=debug_mode;
}

std::vector <std::string> nlu::tokenize(std::string &input)
{
	tokenizer * tokenizer_tmp = new tokenizer(_lang,false);
	std::vector <std::string> to_return = tokenizer_tmp->tokenize(input);
	delete(tokenizer_tmp);
	return to_return;
}

std::string nlu::tokenize_str(std::string &input)
{
	tokenizer * tokenizer_tmp = new tokenizer(_lang,false);
	std::string to_return = tokenizer_tmp->tokenize_str(input);
	delete(tokenizer_tmp);
	return to_return;
}
