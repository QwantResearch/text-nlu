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

nlu::nlu(int debug_mode)
{
    std::cout << "NLU Init" << std::endl;
    std::string channel("tensorflow_serving:8500");
    _stub = PredictionService::NewStub(CreateChannel(channel, grpc::InsecureChannelCredentials()));
    // _stub = PredictionService::PredictionService::NewStub(CreateChannel(channel, grpc::InsecureChannelCredentials()));
    std::cout << "NLU Init done" << std::endl;
    _local=true;
    _debug_mode=debug_mode;
}

std::vector<std::string> nlu::getDomains(){
  //TODO (calling tfserving?)
  //TODO test: _stub->GetModelMetadata(); to get model availables ?
  // PredictRequest request2;
  // request2.mutable_model_spec()->set_name("domain");

  std::vector<std::string> domain_list;
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

bool nlu::NLUBatch(
    std::vector<std::vector<std::string> >& batch_tokens,
    std::vector<std::vector<std::string> >& output_batch_tokens,
    std::string domain) {
  // Pad batch.
  std::vector<int> lengths = PadBatch(batch_tokens);

  long batch_size = batch_tokens.size();
  long max_length = batch_tokens.front().size();
  
  std::vector<std::vector<std::vector<std::string> > > chars_list_bis_batch;
  std::vector<std::vector<std::vector<std::string> > > chars_list_batch;
  std::vector<std::vector<std::string> > chars_list;
  std::vector<std::string> l_tokens_in_batch ;
  
  int l_inc=0;
  int l_inc_batch=0;
  int l_inc_char=0;
  
  int length = 0;
  int max_length_word = 0;
  for (l_inc_batch = 0; l_inc_batch < batch_size; l_inc_batch++)
  {
      l_tokens_in_batch = batch_tokens.at(l_inc_batch);
      length = l_tokens_in_batch.size();
      for (l_inc = 0 ; l_inc < length; l_inc++)
      {
          string word = l_tokens_in_batch.at(l_inc);
          if (max_length_word < (int)word.size())
          {
              max_length_word = (int)word.size();
          }
      }
  }
  for (l_inc_batch = 0; l_inc_batch < batch_size; l_inc_batch++)
  {
      l_tokens_in_batch = batch_tokens.at(l_inc_batch);
      length = l_tokens_in_batch.size();
      std::vector<std::vector<std::string> > chars_list_bis;
      for (l_inc = 0 ; l_inc < length; l_inc++)
      {
          vector<std::string> l_char;
          std::string word = l_tokens_in_batch.at(l_inc);
          for (l_inc_char = 0 ; l_inc_char < max_length_word; l_inc_char++)
          {
              if (l_inc_char < (int)word.size())
              {
                  stringstream l_ss;
                  l_ss << word[l_inc_char];
                  l_char.push_back(l_ss.str());
              }
              else
              {
                  l_char.push_back("");
              }
          }
          chars_list.push_back(l_char);
      }

    chars_list_batch.push_back(chars_list);

    l_inc=0;  
    while (l_inc < max_length_word)
    {
        std::vector<std::string> l_vectmp;
        chars_list_bis.push_back(l_vectmp);
        int l_inc_bis=0;
        while (l_inc_bis < length)
        {
            chars_list_bis[l_inc].push_back(chars_list[l_inc_bis][l_inc]);
            l_inc_bis=l_inc_bis+1;
        }
        l_inc=l_inc+1;
    }
    chars_list_bis_batch.push_back(chars_list_bis);
  }

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
  std::cout << "Generate tokens_tensor ok." << std::endl;

  // PROTO: chars_tensor
  tensorflow::TensorProto chars_tensor;
  chars_tensor.set_dtype(tensorflow::DataType::DT_STRING);

  //TODO: check consistency
  for (int i = 0; i < batch_size; i++) {
      for (int j = 0; j < max_length; j++) { 
          for (int k = 0; k < max_length_word; k++) {
            chars_tensor.add_string_val(chars_list_batch[i][j][k]);  //TODO: check if bis or not?
          }
      }
  }

  chars_tensor.mutable_tensor_shape()->add_dim()->set_size(batch_size);
  chars_tensor.mutable_tensor_shape()->add_dim()->set_size(max_length);
  chars_tensor.mutable_tensor_shape()->add_dim()->set_size(max_length_word);

  inputs["chars"] = chars_tensor;
  std::cout << "Generate chars_tensor ok." << std::endl;

  // PROTO: lengths_tensor
  tensorflow::TensorProto lengths_tensor;
  lengths_tensor.set_dtype(tensorflow::DataType::DT_INT32);

  for (int i = 0; i < lengths.size(); i++) {
    lengths_tensor.add_int_val(lengths[i]);
  }
  lengths_tensor.mutable_tensor_shape()->add_dim()->set_size(batch_size);

  inputs["length"] = lengths_tensor;
  std::cout << "Generate lengths_tensor ok." << std::endl;

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
      } else {
        std::cout << "other section: " << section << std::endl;
      }
      ++output_index;
    }
   
  } else {
    std::cout << "gRPC call return code: " << status.error_code() << ": "
              << status.error_message() << std::endl;
    return "RPC failed";
  }

  return true;
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
