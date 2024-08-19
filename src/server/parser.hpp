#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "server.hpp"

enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };

struct Page {
  bool requireSSL;
  std::function<std::string(void)> *methods[8];
};

class Router
{
public:
  Router(std::string _execPath);

  void add(Method _method, std::string _path, std::function<std::string(void)> *_lambda,
           bool _needSSL = false);
  std::string respond(std::string _message, Client::Type _clientType) const;
  std::string failed() const;

private:
  std::string execPath_;
  std::unordered_map<std::string, Page> paths_;
};

struct Message {
  struct Command {
    Method method;
    std::string path;
    std::string protocol;
  };

  enum Failure{ NONE, HEADER };

  Command cmd;
  std::vector<std::string> headers;
  std::string body;
  Failure failure;
};

Message parse(std::string _message);
