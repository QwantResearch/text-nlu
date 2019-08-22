// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "classifier.h"

std::vector<std::pair<fasttext::real, std::string>>
classifier::prediction(std::string &text, std::string &tokenized, int count, float threshold) {
  std::vector<std::pair<fasttext::real, std::string>> results;
  if (*(text.end() - 1) != '\n')
    text.push_back('\n');
  tokenizer * tokenizer_tmp=new tokenizer(_lang,true);
  tokenized=tokenizer_tmp->tokenize_str(text);
  delete(tokenizer_tmp);

  std::stringstream istr(tokenized);
  _model.predictLine(istr, results, count, threshold);

  for (auto &r : results) {
    // FastText returns labels like : '__label__XX'
    r.second = r.second.substr(9);
  }
  return results;
}
