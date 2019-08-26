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


nlu::nlu(const tensorflow::string& export_dir,std::string& model_domain_param,std::string& lang)
{
    LoadModel(export_dir,model_domain_param,lang);
    _local=true;
    _debug_mode=0;
}


nlu::nlu(shared_ptr<Channel> channel,std::string& model_domain_param,std::string& lang)
{
    LoadModel(channel,model_domain_param,lang);
    _local=false;
    _debug_mode=0;
}


nlu::nlu()
{
    cerr << "Warning: NLU object is empty" <<endl;
    _local=true;
    _debug_mode=0;
}



bool nlu::LoadModel(shared_ptr<Channel> channel,std::string& model_domain_param,std::string& lang)
{
    _domain=model_domain_param;
    stub_=PredictionService::NewStub(channel);
    _local=false;
    _debug_mode=0;
    _lang=lang;
    _tokenizer=new tokenizer(lang,false);
    return true;
}

// Displays a batch of tokens.
void nlu::PrintBatch(
    const std::vector<std::vector<tensorflow::string> >& batch_tokens) {
  for (const auto& tokens : batch_tokens) {
    for (const auto& token : tokens) {
      std::cout << " " << token;
    }
    std::cout << std::endl;
  }
}

// Pads a batch of tokens and returns the length of each sequence.
std::vector<tensorflow::int32> nlu::PadBatch(
    std::vector<std::vector<tensorflow::string> >& batch_tokens) {
  std::vector<tensorflow::int32> lengths;
  size_t max_length = 0;

  lengths.reserve(batch_tokens.size());

  for (const auto& tokens : batch_tokens) {
    lengths.push_back(static_cast<tensorflow::int32>(tokens.size()));
    max_length = std::max(max_length, tokens.size());
  }
  for (auto& tokens : batch_tokens) {
    tokens.insert(tokens.end(), max_length - tokens.size(), "");
  }

  return lengths;
}


// Loads a saved model.
bool nlu::LoadModel(const tensorflow::string& export_dir,std::string& model_domain_param,std::string& lang) {
  tensorflow::SessionOptions session_options;
  tensorflow::RunOptions run_options;

  tensorflow::Status load_saved_model_status =
      LoadSavedModel(session_options, run_options, export_dir,
                     {tensorflow::kSavedModelTagServe}, &bundle);

  if (!load_saved_model_status.ok()) {
    std::cerr << load_saved_model_status << std::endl;
    return false;
  }
  _local=true;
  _domain=model_domain_param;
  _lang=lang;
  _tokenizer=new tokenizer(lang,false);
  return true;
}

// Process a batch of tokenizes sentences.
bool nlu::NLUBatch(
    std::vector<std::vector<tensorflow::string> >& batch_tokens,
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens) {
  // Pad batch.
  std::vector<tensorflow::int32> lengths = PadBatch(batch_tokens);

  tensorflow::int64 batch_size = batch_tokens.size();
  tensorflow::int64 max_length = batch_tokens.front().size();
  
  std::vector<std::vector<std::vector<tensorflow::string> > > chars_list_bis_batch;
  std::vector<std::vector<tensorflow::string> > chars_list;
  std::vector<tensorflow::string> l_tokens_in_batch ;
  
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
      std::vector<std::vector<tensorflow::string> > chars_list_bis;
      for (l_inc = 0 ; l_inc < length; l_inc++)
      {
          vector<tensorflow::string> l_char;
          tensorflow::string word = l_tokens_in_batch.at(l_inc);
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
 

    l_inc=0;  
    while (l_inc < max_length_word)
    {
        std::vector<tensorflow::string> l_vectmp;
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

  // Convert to tensors.
  std::vector<tensorflow::string> flat_batch_tokens = FlattenVector(batch_tokens);
  std::vector<tensorflow::string> flat_batch_char = FlattenVector(chars_list_bis_batch);
  tensorflow::Tensor tokens_tensor  =  AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor chars_tensor   = AsTensor(flat_batch_char, {batch_size, max_length, max_length_word});
  tensorflow::Tensor lengths_tensor = AsTensor(lengths);

  // Resolve name of inputs to fed and outputs to fetch.
  const auto signature_def_map = bundle.meta_graph_def.signature_def();
  const auto signature_def = signature_def_map.at(tensorflow::kDefaultServingSignatureDefKey);
  const tensorflow::string tokens_input_name  = signature_def.inputs().at("tokens").name();
  const tensorflow::string char_input_name    = signature_def.inputs().at("chars").name();
  const tensorflow::string length_input_name  = signature_def.inputs().at("length").name();
  const tensorflow::string tokens_output_name = signature_def.outputs().at("tags").name();
  const tensorflow::string length_output_name = signature_def.outputs().at("length").name();

  // Forward in the graph.
  std::vector<tensorflow::Tensor> outputs;
  tensorflow::Status run_status = bundle.session->Run(
      {{tokens_input_name, tokens_tensor}, {char_input_name, chars_tensor}, {length_input_name, lengths_tensor}},
      {tokens_output_name, length_output_name}, {}, &outputs);

  if (!run_status.ok()) {
    std::cerr << "Running model failed: " << run_status << std::endl;
    return false;
  }

  // Convert TensorFlow tensors to Eigen tensors.
  auto e_tags = outputs[0].tensor<tensorflow::string,2>();
  auto e_length = outputs[1].tensor<tensorflow::int32,1>();
// 
  // Collect results in C++ vectors.
  for (long b = 0; b < batch_size; ++b) {
    long len = e_length(b);
    std::vector<tensorflow::string> output_tokens;
    output_tokens.reserve(len);
    for (long i = 0; i < len ; ++i) {
      output_tokens.push_back(e_tags(b, i));
    }
    output_batch_tokens.push_back(output_tokens);
  }

  return true;
}


// Process a batch of tokenizes sentences.
bool nlu::NLUBatchOnline(
    std::vector<std::vector<tensorflow::string> >& batch_tokens,
    std::vector<std::vector<tensorflow::string> >& output_batch_tokens) {
  // Pad batch.
  std::vector<tensorflow::int32> lengths = PadBatch(batch_tokens);

  tensorflow::int64 batch_size = batch_tokens.size();
  tensorflow::int64 max_length = batch_tokens.front().size();
  
  std::vector<std::vector<std::vector<tensorflow::string> > > chars_list_bis_batch;
  std::vector<std::vector<tensorflow::string> > chars_list;
  std::vector<tensorflow::string> l_tokens_in_batch ;
  
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
      std::vector<std::vector<tensorflow::string> > chars_list_bis;
      for (l_inc = 0 ; l_inc < length; l_inc++)
      {
          vector<tensorflow::string> l_char;
          tensorflow::string word = l_tokens_in_batch.at(l_inc);
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
 

    l_inc=0;  
    while (l_inc < max_length_word)
    {
        std::vector<tensorflow::string> l_vectmp;
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

  // Convert to tensors.
  tensorflow::Scope l_scope = tensorflow::Scope::NewRootScope();
  std::vector<tensorflow::string> flat_batch_tokens = FlattenVector(batch_tokens);
  std::vector<tensorflow::string> flat_batch_char = FlattenVector(chars_list_bis_batch);
  tensorflow::Tensor tokens_tensor  =  AsTensor(flat_batch_tokens, {batch_size, max_length});
  tensorflow::Tensor chars_tensor   = AsTensor(flat_batch_char, {batch_size, max_length, max_length_word});
  tensorflow::Tensor lengths_tensor = AsTensor(lengths);
  tensorflow::TensorProto* tokens_tensor_proto = new tensorflow::TensorProto();
  tensorflow::TensorProto* chars_tensor_proto = new tensorflow::TensorProto();
  tensorflow::TensorProto* lengths_tensor_proto = new tensorflow::TensorProto();

  PredictRequest predictRequest;
  PredictResponse response;
  ClientContext context;

  predictRequest.mutable_model_spec()->set_name(_domain);

//   google::protobuf::Map< std::string, tensorflow::Tensor >& inputs = *predictRequest.mutable_inputs();
  google::protobuf::Map< std::string, tensorflow::TensorProto >& inputs = *(predictRequest.mutable_inputs());
  tokens_tensor.AsProtoField(tokens_tensor_proto);
  chars_tensor.AsProtoField(chars_tensor_proto);
  lengths_tensor.AsProtoField(lengths_tensor_proto);
  
//   =tensorflow::ops::SerializeTensor(l_scope,tokens_tensor);
//   inputs["tokens"]=::Output();
//   inputs["chars"]=tensorflow::ops::SerializeTensor(l_scope ,chars_tensor);
//   inputs["length"]=tensorflow::ops::SerializeTensor(l_scope ,lengths_tensor);
  inputs["tokens"]=(*tokens_tensor_proto);
  inputs["chars"]=(*chars_tensor_proto);
  inputs["length"]=(*lengths_tensor_proto);

//       cerr << "Asking Stub though predict !!!" <<endl;

  Status status = stub_->Predict(&context, predictRequest, &response);
  
//   std::cerr << "check status.." << std::endl;
  
  if (status.ok()) 
  {
//     std::cout << "call predict ok" << std::endl;
//     std::cout << "outputs size is "<< response.outputs_size() << std::endl;
    OutMap& map_outputs =  *response.mutable_outputs();
    OutMap::iterator iter;
    int output_index = 0;
//     tensorflow::Tensor tensor_tmp;
    tensorflow::Tensor e_tags;
    tensorflow::Tensor e_length;
    
    for(iter = map_outputs.begin();iter != map_outputs.end(); ++iter)
    {
      tensorflow::TensorProto& result_tensor_proto= iter->second;
      tensorflow::Tensor tensor;

      bool converted = tensor.FromProto(result_tensor_proto);
      if (converted) {
        if (iter->first.find("tags") == 0) e_tags.FromProto(result_tensor_proto);
        if (iter->first.find("length") == 0) e_length.FromProto(result_tensor_proto);
//         std::cout << "the " <<iter->first <<" result tensor[" << output_index << "] is:" <<
//               std::endl << tensor.SummarizeValue(250) << std::endl;
      }else {
        std::cerr << "the " <<iter->first <<" result tensor[" << output_index << 
              "] convert failed." << std::endl;
      }
      ++output_index;
    }
// 
  // Collect results in C++ vectors.
  
  long prev=0;
  vector<string> vec_length;
  vector<string> vec_tags;
  Split_str(e_length.SummarizeValue(250),vec_length, " ");
  Split_str(e_tags.SummarizeValue(250),vec_tags, " ");
  for (long b = 0; b < batch_size; ++b) {
    long len = prev+atol(vec_length[b].c_str());
//     prev=
    std::vector<tensorflow::string> output_tokens;
    output_tokens.reserve(len);
    for (long i = prev; i < len ; ++i) {
      output_tokens.push_back(vec_tags[i]);
    }
    prev=len;
    output_batch_tokens.push_back(output_tokens);
  }  


  return "Done.";
  } else {
    std::cout << "gRPC call return code: " 
        <<status.error_code() << ": " << status.error_message()
        << std::endl;
    return "gRPC failed.";
  }


  return true;
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
