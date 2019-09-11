// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.
#include "nlu.h"

using namespace std;

bool nlu::getLocal()
{
    return _local;
}

nlu::nlu(int debug_mode, std::string model_config_path, std::string tfserving_host)
{
    std::string channel(tfserving_host);
    _stub = PredictionService::NewStub(CreateChannel(channel, grpc::InsecureChannelCredentials()));
    _local=true;
    _debug_mode=debug_mode;
    _model_config_path = model_config_path;
    if (debug_mode){
      cerr << "[DEBUG]\t" << currentDateTime() << "\tNLU initialized successfully." << std::endl;
    }
}

std::vector<std::string> nlu::getDomains(){
  // We can't get the list of models from tfserving. 
  // See: https://github.com/tensorflow/serving/pull/797
  // Instead we read it directly from models.config.
  // As it is possible to change the config file on the fly,
  // we don't store the retrieved information.

  std::vector<std::string> domain_list;

  tensorflow::serving::ModelServerConfig *server_config = new tensorflow::serving::ModelServerConfig();

  int fileDescriptor = open(_model_config_path.c_str(), O_RDONLY);
  if( fileDescriptor < 0 ) {
    cerr << "[ERROR]\t" << currentDateTime() << "\tError opening the file " << std::endl;
    return domain_list;
  }

  google::protobuf::io::FileInputStream fileInput(fileDescriptor);
  fileInput.SetCloseOnDelete( true );

  if (!google::protobuf::TextFormat::Parse(&fileInput, server_config)) {
    cerr << "[ERROR]\t" << currentDateTime() << "\tFailed to parse file!" << endl;
    return domain_list;
  }

  const tensorflow::serving::ModelConfigList list = server_config->model_config_list();
  for (int index = 0; index < list.config_size(); index++) {
    const tensorflow::serving::ModelConfig config = list.config(index);
    domain_list.push_back(config.name());
  }

  return domain_list;
}

std::vector<int> nlu::PadBatch(
    std::vector<std::vector<std::string> >& batch_tokens) {
  std::vector<int> lengths;
  size_t max_length = 0;

  lengths.reserve(batch_tokens.size());

  for (const auto& tokens : batch_tokens) {
    lengths.push_back(tokens.size());
    max_length = std::max(max_length, tokens.size());
  }
  for (auto& tokens : batch_tokens) {
    tokens.insert(tokens.end(), max_length - tokens.size(), "");
  }

  return lengths;
}

int nlu::getMaxLengthWord(std::vector<std::vector<std::string>>& batch_tokens) {
  int max_length_word = 0;

  for (auto &tokens: batch_tokens) {
    for (auto &token: tokens) {
      max_length_word = std::max(max_length_word, (int)token.size());
    }
  }
  return max_length_word;
}

void nlu::getBatchCharsListFromBatchTokens(
  std::vector<std::vector<std::vector<std::string>>>& batch_chars_list,
  std::vector<std::vector<std::string>>& batch_tokens, 
  int max_length_word){
  // We construct batch_chars_list from batch_tokens
  // by pushing tokens chars by chars and padding to max_length_word.

  for (auto &tokens: batch_tokens) {
    std::vector<std::vector<std::string> > tokens_chars_list;
    for (auto &token: tokens) {
      std::vector<std::string> token_chars_list;

      for (auto &current_char: token){
        token_chars_list.push_back((std::string)&current_char);
      }
      //Padding to max_length_word
      token_chars_list.insert(token_chars_list.end(), max_length_word - token_chars_list.size(), "");

      tokens_chars_list.push_back(token_chars_list);
    }
    batch_chars_list.push_back(tokens_chars_list);
  }
}

Status nlu::NLUBatch(
    std::vector<std::vector<std::string> >& batch_tokens,
    std::vector<std::vector<std::string> >& output_batch_tokens,
    std::string domain) {

  // Pad batch.
  std::vector<int> lengths = PadBatch(batch_tokens);

  long batch_size = batch_tokens.size();
  long max_length = batch_tokens.front().size();

  int max_length_word = getMaxLengthWord(batch_tokens);

  std::vector<std::vector<std::vector<std::string> > > batch_chars_list;
  getBatchCharsListFromBatchTokens(batch_chars_list, batch_tokens, max_length_word);

  // Data we are sending to the server.
  PredictRequest request;
  request.mutable_model_spec()->set_name(domain);

  // Container for the data we expect from the server.
  PredictResponse response;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  google::protobuf::Map<std::string, tensorflow::TensorProto>& inputs =
    *request.mutable_inputs();


  // PROTO: tokens_tensor
  tensorflow::TensorProto tokens_tensor;
  tokens_tensor.set_dtype(tensorflow::DataType::DT_STRING);
  for (int i = 0; i < batch_size; i++) {
      for (int j = 0; j < max_length; j++) {
          tokens_tensor.add_string_val(batch_tokens[i][j]);  
      }  
  }
  tokens_tensor.mutable_tensor_shape()->add_dim()->set_size(batch_size);
  tokens_tensor.mutable_tensor_shape()->add_dim()->set_size(max_length);

  inputs["tokens"] = tokens_tensor;

  // PROTO: chars_tensor
  tensorflow::TensorProto chars_tensor;
  chars_tensor.set_dtype(tensorflow::DataType::DT_STRING);

  for (int i = 0; i < batch_size; i++) {
      for (int j = 0; j < max_length; j++) { 
          for (int k = 0; k < max_length_word; k++) {
            chars_tensor.add_string_val(batch_chars_list[i][j][k]);
          }
      }
  }

  chars_tensor.mutable_tensor_shape()->add_dim()->set_size(batch_size);
  chars_tensor.mutable_tensor_shape()->add_dim()->set_size(max_length);
  chars_tensor.mutable_tensor_shape()->add_dim()->set_size(max_length_word);

  inputs["chars"] = chars_tensor;

  // PROTO: lengths_tensor
  tensorflow::TensorProto lengths_tensor;
  lengths_tensor.set_dtype(tensorflow::DataType::DT_INT32);

  for (int i = 0; i < lengths.size(); i++) {
    lengths_tensor.add_int_val(lengths[i]);
  }
  lengths_tensor.mutable_tensor_shape()->add_dim()->set_size(batch_size);

  inputs["length"] = lengths_tensor;

  // The actual RPC.
  Status status = _stub->Predict(&context, request, &response);
  
  // Act upon its status.
  if (status.ok()) {

    OutMap& map_outputs = *response.mutable_outputs();
    OutMap::iterator iter;
    int output_index = 0;

    for (iter = map_outputs.begin(); iter != map_outputs.end(); ++iter) {
      tensorflow::TensorProto& result_tensor_proto = iter->second;
      std::string section = iter->first;

      int current_index = 0;
      if ("tags" == section) {
        for (int it=0; it < batch_size; it++){
          std::vector<std::string> output_tokens;
          for (int l=0; l < lengths[it]; l++){ //TODO: maybe use the "lenghts" value returned instead
            output_tokens.push_back(result_tensor_proto.string_val(current_index));
            current_index++;
          }
          output_batch_tokens.push_back(output_tokens);
          current_index++;
        }
      }
      ++output_index;
    }
   
  } else {
    cerr << "[ERROR]\t" << currentDateTime() << "\tError: gRPC call return code: " 
         << status.error_code() << ": "
         << status.error_message() << std::endl;

    if (status.error_code() == grpc::StatusCode::NOT_FOUND)
      return Status(grpc::StatusCode::NOT_FOUND, "NLU model not found");
    return Status(grpc::StatusCode::INTERNAL, "Tensorflow Serving prediction failed");
  }
  return status;
}

void nlu::setDebugMode(int debug_mode)
{
    _debug_mode=debug_mode;
}

std::vector <std::string> nlu::tokenize(std::string &input, std::string lang)
{
	tokenizer * tokenizer_tmp = new tokenizer(lang,false);
	std::vector <std::string> to_return = tokenizer_tmp->tokenize(input);
	delete(tokenizer_tmp);
	return to_return;
}

std::string nlu::tokenize_str(std::string &input, std::string lang)
{
	tokenizer * tokenizer_tmp = new tokenizer(lang,false);
	std::string to_return = tokenizer_tmp->tokenize_str(input);
	delete(tokenizer_tmp);
	return to_return;
}
