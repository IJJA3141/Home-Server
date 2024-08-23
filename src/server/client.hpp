#pragma once

#include <map>
#include <string>
#include <vector>

static const int method_size = 8;
enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };

struct Command {
  Method method;
  std::vector<std::string> path;
  std::string protocol;
};

class Request
{
public:
  static const int failure_size = 5;
  enum Failure { NONE, HEADER, LENGTH, PATH, MOVE };

  Failure failure;
  Command cmd;
  std::map<std::string, std::string> headers;
  std::string body;

  Request(const std::string _req);

private:
  class Stream
  {
    public:
    Stream(const std::string _string);

    bool operator>>(std::string &_string);

    private:
    std::string string_;
    size_t begin_;
    size_t end_;
    size_t 
  };
};

class Response
{
};

class Client
{
};

class SSLClient : public Client
{
};
