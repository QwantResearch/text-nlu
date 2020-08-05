// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "rest_server.h"
#include "nlu.h"
#include "utils.h"


void rest_server::init(size_t thr) {
  // Creating the entry point of the REST API.
  Pistache::Port pport(_num_port);
  Address addr(Ipv4::any(), pport);
  httpEndpoint = std::make_shared<Http::Endpoint>(addr);

  auto opts = Http::Endpoint::options().threads(thr).flags(
      Tcp::Options::InstallSignalHandler);
  httpEndpoint->init(opts);
  setupRoutes();
}

void rest_server::start() {
  httpEndpoint->setHandler(router.handler());

  cout << "[INFO]\t" << currentDateTime() <<"\tREST server listening on 0.0.0.0:" << _num_port << endl;
  httpEndpoint->serve();

  httpEndpoint->shutdown();
}

void rest_server::setupRoutes() {
  using namespace Rest;

  Routes::Post(router, "/nlu/",
               Routes::bind(&rest_server::doNLUPost, this));

  Routes::Post(router, "/nlu_batch/",
              Routes::bind(&rest_server::doNLUBatchPost, this));

  Routes::Get(router, "/nlu/",
              Routes::bind(&rest_server::doNLUGet, this));
}


void rest_server::doNLUGet(const Rest::Request &request, Http::ResponseWriter response) {
  response.headers().add<Http::Header::AccessControlAllowHeaders>(
      "Content-Type");
  response.headers().add<Http::Header::AccessControlAllowMethods>(
      "GET, POST, DELETE, OPTIONS");
  response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
  response.headers().add<Http::Header::ContentType>(MIME(Application, Json));

  std::vector<std::string> list_nlu = _nlu->getDomains();

  string response_string = "{\"nlu-domains\":[";
  for (int inc = 0; inc < (int)list_nlu.size(); inc++) {
    if (inc > 0)
      response_string.append(",");
    response_string.append("\"");
    response_string.append(list_nlu.at(inc));
    response_string.append("\"");
  }
  response_string.append("]}");
  if (_debug_mode != 0)
    cerr << "[DEBUG]\t" << currentDateTime() << "\tRESPONSE\t" << response_string << endl;
  response.send(Pistache::Http::Code::Ok, response_string);
}

void rest_server::doNLUPost(const Rest::Request &request, Http::ResponseWriter response) {
  response.headers().add<Http::Header::AccessControlAllowHeaders>(
      "Content-Type");
  response.headers().add<Http::Header::AccessControlAllowMethods>(
      "GET, POST, DELETE, OPTIONS");
  response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
  nlohmann::json j = nlohmann::json::parse(request.body());

  int count;
  float threshold;
  bool debugmode;
  bool batchmode;
  bool detokenization;
  bool lowercase;
  string domain;
  string lang;
  
  auto err = rest_server::fetchParamWithDefault(j, domain, lang, count, threshold, debugmode,batchmode, detokenization, lowercase);
  if (err != NULL) {
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Bad_Request, err);
  }

  if (j.find("text") != j.end()) {
    string text = j["text"];
    string tokenized;
    if (_debug_mode != 0)
      cerr << "[DEBUG]\t" << currentDateTime() << "\t" << "ASK NLU:\t" << j << endl;
    Status status = askNLU(text, tokenized, j, domain, lang, debugmode,batchmode, detokenization,lowercase);
    if (!status.ok()) {
      response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
      response.send(Http::Code::Internal_Server_Error, std::string(status.error_message()));
    }
    j.push_back(nlohmann::json::object_t::value_type(string("tokenized"), tokenized));

    std::string s = j.dump();
    if (_debug_mode != 0)
      cerr << "[DEBUG]\t" << currentDateTime() << "\tRESPONSE\t" << s << endl;
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Ok, std::string(s));
  } else {
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Bad_Request,std::string("The `text` value is required"));
  }
}

void rest_server::doNLUBatchPost(const Rest::Request &request, Http::ResponseWriter response) {
  response.headers().add<Http::Header::AccessControlAllowHeaders>(
      "Content-Type");
  response.headers().add<Http::Header::AccessControlAllowMethods>(
      "GET, POST, DELETE, OPTIONS");
  response.headers().add<Http::Header::AccessControlAllowOrigin>("*");
  nlohmann::json j = nlohmann::json::parse(request.body());

  int count;
  float threshold;
  bool debugmode;
  bool batchmode;
  bool detokenization;
  bool lowercase;
  string domain;
  string lang;

  auto err = rest_server::fetchParamWithDefault(j, domain, lang, count, threshold, debugmode,batchmode, detokenization, lowercase);
  if (err != NULL) {
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Bad_Request, err);
  }

  if (j.find("batch_data") != j.end()) {
    for (auto& it: j["batch_data"]){
      if (it.find("text") != it.end()) {
        string text = it["text"];
        string tokenized;
        if (_debug_mode != 0)
          cerr << "[DEBUG]\t" << currentDateTime() << "\tASK NLU:\t" << it << endl;
        Status status = askNLU(text, tokenized, it, domain, lang, debugmode,batchmode, detokenization, lowercase);
        if (!status.ok()) {
          response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
          response.send(Http::Code::Internal_Server_Error, std::string(status.error_message()));
        }
        it.push_back(nlohmann::json::object_t::value_type(string("tokenized"), tokenized));
      } else {
        response.headers().add<Http::Header::ContentType>(
            MIME(Application, Json));
        response.send(Http::Code::Bad_Request,
                      std::string("`text` value is required for each item in `batch_data` array"));
      }
    }
    std::string s = j.dump();
    if (_debug_mode != 0)
      cerr << "[DEBUG]\t" << currentDateTime() << "\tRESULT NLU:\t" << s << endl;
    response.headers().add<Http::Header::ContentType>(
        MIME(Application, Json));
    response.send(Http::Code::Ok, std::string(s));
  } else {
    response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
    response.send(Http::Code::Bad_Request,
                  std::string("`batch_data` value is required"));
  }
}


const char *rest_server::fetchParamWithDefault(
  const nlohmann::json& j,
  string& domain, string& lang,
  int& count,
  float& threshold,
  bool& debugmode,
  bool& batch,
  bool& detok,
  bool& lowercase
) {
  count = 10;
  threshold = 0.0;
  debugmode = false;
  batch = false;
  detok = false;
  lowercase = false;

  if (j.find("count") != j.end()) {
    count = j["count"];
  }
  if (j.find("lang") != j.end()) {
    lang = j["lang"];
  } else {
    return "`lang` value is null";
  }
  if (j.find("threshold") != j.end()) {
    threshold = j["threshold"];
  }
  if (j.find("debug") != j.end()) {
    debugmode = j["debug"];
  }
  if (j.find("detok") != j.end()) {
    detok = j["detok"];
  }
  if (j.find("lowercase") != j.end()) {
    lowercase = j["lowercase"];
  }
  if (j.find("batch") != j.end()) {
    batch = j["batch"];
  }
  if (j.find("domain") != j.end()) {
    domain = j["domain"];
  } else {
    return "`domain` value is null";
  }
  return NULL;
}

Status rest_server::askNLU(
  std::string &text,
  std::string &tokenized_text,
  json &output,
  string &domain,
  string &lang,
  bool debugmode,
  bool batchmode,
  bool detokenization,
  vool lowercase
) {
  tokenized_text = _nlu->tokenize_str(text, lang, lowercase);
  std::vector<std::string> tokenized_vec = _nlu->tokenize(text, lang, lowercase);

  vector<vector<string> > tokenized_batched;
  vector<string> tokenized_vec_tmp;
  for (int l_inc=0; l_inc < (int)tokenized_vec.size(); l_inc++) {
    tokenized_vec_tmp.push_back(tokenized_vec[l_inc]);
    if (batchmode && (l_inc == (int)tokenized_vec.size()-1 || ((int)tokenized_vec[l_inc].size() == 1 && (tokenized_vec[l_inc].compare(".")==0 || tokenized_vec[l_inc].compare("\n")==0))))
    {
        tokenized_batched.push_back(tokenized_vec_tmp);
        tokenized_vec_tmp.clear();
    }
  }
  if ((int)tokenized_vec_tmp.size() > 0) {
    tokenized_batched.push_back(tokenized_vec_tmp);
  }
//     if (_debug_mode != 0 ) cerr << "LOG: "<< currentDateTime() << "\t" << "BATCH SIZE:\t" << (int)tokenized_batched.size() << endl;
  if (_debug_mode != 0)
    cerr << "LOG: "<< currentDateTime() << "\t" << "BATCH CONTENT:\t" << printBatch(tokenized_batched) << endl;
  return askNLU(tokenized_batched, output, domain, lang, debugmode, detokenization, lowercase);
}

Status rest_server::askNLU(
  vector<vector<string> > &input,
  json &output,
  string &domain,
  string &lang,
  bool debugmode,
  bool detokenization,
  bool lowercase
) {
  vector<vector<string> > result_batched ;
  Status status = _nlu->NLUBatch(input,result_batched, domain);
  if (_debug_mode != 0)
    cerr << "LOG: "<< currentDateTime() << "\t" << "BATCH OUTPUT:\t" << printBatch(result_batched) << endl;
  if (!status.ok()) {
    return status;
  }

  json i_tmp = json::array();
  json k_tmp = json::array();
  json t_tmp;
  string prev_tag("");
  string curr_tag("");
  string word_concat("");
  for (int i = 0; i < (int)input.size(); i++) {
    for (int j = 0;j < (int)input.at(i).size(); j++) {
      json j_tmp;
      bool begin_tag=0;
      curr_tag=result_batched.at(i).at(j);
      if (curr_tag.find("I-") == 0) {
        curr_tag=curr_tag.substr(2);
      }
      if (curr_tag.find("B-") == 0) {
        begin_tag=1;
        curr_tag=curr_tag.substr(2);
      }
      j_tmp.push_back(nlohmann::json::object_t::value_type(string("word"), input.at(i).at(j)));
      j_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), result_batched.at(i).at(j)));
      j_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), string("")));
      i_tmp.push_back(j_tmp);

      if ((int)prev_tag.length() > 0 && (prev_tag.compare(curr_tag) != 0 || begin_tag)) {
          if (detokenization) word_concat = _nlu->detokenize_str(word_concat, lang);
          t_tmp.push_back(nlohmann::json::object_t::value_type(string("phrase"), word_concat ));
          t_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), prev_tag ));
          json j_value({});
          t_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), j_value));
          word_concat=input.at(i).at(j);
          k_tmp.push_back(t_tmp);
          t_tmp.clear();
      } else {
        if (word_concat.length() > 0)
          word_concat.append(" ");
        word_concat.append(input.at(i).at(j));
      }
      prev_tag=curr_tag;
    }
  }
  if ((int)word_concat.length() > 0) {
    if (detokenization) word_concat = _nlu->detokenize_str(word_concat, lang);
    t_tmp.push_back(nlohmann::json::object_t::value_type(string("phrase"), word_concat ));
    t_tmp.push_back(nlohmann::json::object_t::value_type(string("tag"), prev_tag ));
    json j_value({});
    t_tmp.push_back(nlohmann::json::object_t::value_type(string("value"), j_value));
    k_tmp.push_back(t_tmp);
  }
  if (debugmode)
    output.push_back(nlohmann::json::object_t::value_type(string("NLU_DEBUG"), i_tmp));
  output.push_back(nlohmann::json::object_t::value_type(string("NLU"), k_tmp));
  return status;
}


void rest_server::doAuth(const Rest::Request &request, Http::ResponseWriter response) {
  printCookies(request);
  response.cookies().add(Http::Cookie("lang", "fr-FR"));
  response.send(Http::Code::Ok);
}

void rest_server::shutdown() {
  httpEndpoint->shutdown();
}

std::string rest_server::printBatch(vector<vector<std::string> > &batchVector) {
  stringstream to_return;

  to_return << "BATCH SIZE: "<<(int)batchVector.size() <<endl;
  for (int l_inc = 0; l_inc < (int)batchVector.size(); l_inc++) {
    to_return << "\tBATCH " << l_inc << "\t[";
    for (int l_inc_sec = 0; l_inc_sec < (int)batchVector[l_inc].size(); l_inc_sec++) {
      if (l_inc_sec > 0)
        to_return << ", ";
      to_return << "\""<< batchVector[l_inc][l_inc_sec] <<"\"";
    }
    to_return << "]" << endl;
  }
  return to_return.str();
}
