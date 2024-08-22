#pragma once

#include "parser.hpp"
#include "server.hpp"

#include <functional>
#include <string>

struct Path {
  std::string match;
  std::vector<std::pair<int, std::string>> skip;
  std::function<std::string(void)> *methods[8];
  std::function<std::string(void)> *SSLMethods[8];

  Path(std::string _path);
};

class Router
{
public:
  Router(std::string _execPath);

  void add(const Method _method, std::string _path, std::function<std::string(void)> *_lambda,
           bool _needSSL = false);
  std::string respond(Message _message, Client::Type _clientType) const;
  std::string failed() const;
  std::string loadFile(std::string _path);

private:
  std::string execPath_;
  std::vector<Path> paths_;
};
