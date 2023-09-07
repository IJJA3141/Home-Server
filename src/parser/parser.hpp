#ifndef PARSER
#define PARSER

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <istream>
#include <streambuf>
#include <string>

namespace http {

enum Method {
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  TRACE,
  PATCH,
};

enum Connection {
  ALL,
  KEEPALIVE,
  CLOSE,
};

struct Header {
  std::string host = "NULL";
  std::string agent = "NULL";
  std::string accept = "NULL";
  std::string lang = "NULL";
  std::string encoding = "NULL";
  http::Connection connection = http::Connection::ALL;
  bool upgrade = false;
  std::string type = "NULL";
  int length = -1;
};

struct Request {
  http::Method method;
  std::string uri;
  std::string file;
  std::string arg;
  std::string version;
  http::Header header;
  std::string body;
};

class Buffer {
public:
  bool next;

  Buffer(std::string _str);
  std::string GetLine(int _skip = 0);

private:
  std::string str_;
  size_t pin_;
  size_t end_;
};

Request Parse(http::Buffer _buffer);

} // namespace http

#endif // !PARSER
