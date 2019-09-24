// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#include "utils.h"

void printCookies(const Pistache::Http::Request &req) {
  auto cookies = req.cookies();
  const std::string indent(4, ' ');

  std::cout << "Cookies: [" << std::endl;
  for (const auto &c : cookies) {
    std::cout << indent << c.name << " = " << c.value << std::endl;
  }
  std::cout << "]" << std::endl;
}

const std::string currentDateTime() {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
  // for more information about date/time format
  strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

  return buf;
}

namespace Generic {
void handleReady(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response) {
  response.send(Pistache::Http::Code::Ok, "1");
}
} // namespace Generic
