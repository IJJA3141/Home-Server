#pragma once

#include <map>
#include <string>
#include <vector>

namespace http
{

enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };
static const int METHOD_SIZE = 8;
static_assert((METHOD_SIZE - 1) == Method::TRACE);

struct Request {
  struct Command {
    Method method;
    std::vector<std::string> path;
    std::string protocol;
  } cmd;

  enum Failure {
    METHOD,
    TRAILING,
    SIZE,
    PATH,
    LENGTH,
    UNAUTHORIZEDMETHOD,
    HEADER,
    MALFORMED,
    PROTOCOL,
    SERVERFULL,
    WRONGPATH,
    NONE,
  } failure;
  static const int FAILURE_SIZE = 12;
  static_assert((FAILURE_SIZE - 1) == Request::Failure::NONE);

  bool secured;
  std::map<std::string, std::string> url_params;
  std::map<std::string, std::string> headers;

  // pair of values cookies
  // 0, 2, 4... names
  // 1, 3, 5... values
  std::vector<std::string> cookies;
  std::string body;

  Request(const std::string_view _req, const bool _sequre);
  Request(const Request::Failure _failure);

  operator std::string() const;

private:
  void parse_cookies(const std::string_view _cookies);
  bool split(const std::string _separator, const std::string_view _str, std::string &_left,
             std::string &_right);
};

struct Response {
  struct Command {
    std::string protocol;
    int status_code;
  } cmd;

  std::map<std::string, std::string> headers;
  std::string body;

  operator std::string() const;
};

} // namespace http
