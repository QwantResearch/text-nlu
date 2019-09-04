// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "abstract_server.h"

AbstractServer::AbstractServer(string &config_file, int debug_mode){
  _debug_mode = debug_mode;

  _nlu = make_shared<nlu>(debug_mode, _model_config_path);
  // TODO: Test if NLU started correctly

  ProcessConfigFile(config_file);
}

void AbstractServer::ProcessConfigFile(std::string &config_file) {

    std::string line;
    int port=9009;
    YAML::Node config;

    try 
    {
    // Reading the configuration file for filling the options.
        config = YAML::LoadFile(config_file);
        cout << "[INFO]\tDomain\t\tLocation/filename\t\tlanguage"<< endl;
        _nbr_threads = config["threads"].as<int>() ;
        _num_port =  config["port"].as<int>() ;
        _debug_mode =  config["debug"].as<int>() ;
        _model_config_path = config["model_config_path"].as<std::string>() ;
    } catch (YAML::BadFile& bf) {
        cerr << "[ERROR]\t" << bf.what() << endl;
        exit(1);
    }
    cout << "[INFO]\tnumber of threads:\t"<< _nbr_threads << endl;
    cout << "[INFO]\tport used:\t"<< _num_port << endl;
    if (_debug_mode > 0) cout << "[INFO]\tDebug mode activated" << endl;
    else cout << "[INFO]\tDebug mode desactivated" << endl;    
}