#pragma once

#include "client.hpp"
#include <map>
#include <string>
#include <vector>

static const int method_size = 8;
enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };
static_assert((method_size - 1) == Method::TRACE, "wrong size for method enum");

class Stream
{
public:
  size_t begin = 0;

  Stream(const std::string _string) : string_(_string), npos(_string.size() - 1) {};

  bool operator>>(std::string &_string);

private:
  std::string string_;
  size_t npos;
  size_t end_ = 0;
};

struct Request {
  struct Command {
    Method method;
    std::vector<std::string> path;
    std::string protocol;
  };
  static const int failure_size = 11;
  enum Failure {
    NONE,
    METHOD,
    TRAILING,
    SIZE,
    PATH,
    LENGTH,
    UNAUTHORIZEDMETHOD,
    HEADER,
    MALFORMED,
    PROTOCOL,
    WRONGPATH,
  };
  static_assert((failure_size - 1) == Request::Failure::WRONGPATH,
                "wrong size for request failure enum");

  Failure failure;
  Request::Command cmd;
  std::map<std::string, std::string> headers;
  std::string body;
  std::map<std::string, std::string> url_args;
  std::map<std::string, std::string> url_params;
  const Client::Type connection_type;

  Request(const std::string _req, const Client::Type _connection_type);
  Request(const Request::Failure _failure = Request::Failure::MALFORMED);
};

struct Response {
  struct Command {
    std::string protocol;
    int status_code;
  };

  Command cmd;
  std::map<std::string, std::string> headers;
  std::string body;

  std::string to_string() const;
};
