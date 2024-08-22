#pragma once

#include <string>
#include <unordered_map>

enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };

struct Message {
  struct Command {
    Method method;
    std::string path;
    std::string protocol;
  };

  enum Failure { NONE, HEADER, LENGTH };

  Command cmd;
  std::unordered_map<std::string, std::string> headers;
  std::string body;
  Failure failure;
};

Message parse(const std::string _message);

class Stream
{
public:
  Stream(std::string _string)
      : string_(_string), begin_(0), end_(0), endOfSteam_(_string.size() - 1){};

  bool operator>>(std::string &_string);

private:
  std::string string_;
  size_t begin_;
  size_t end_;
  size_t endOfSteam_;
};
