#ifndef PARSER
#define PARSER

#include <cstddef>
#include <istream>
#include <streambuf>
#include <string>

namespace http {

enum Method {
  NONE,
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
  std::string host;
  std::string agent;
  std::string accept;
  std::string lang;
  std::string encoding;
  http::Connection connection;
  bool upgrade;
  std::string type;
  int length;
};

struct Request {
  http::Method method;
  std::string uri;
  std::string file;
  std::string version;
  http::Header header;
  std::string body;
};

struct membuf : std::streambuf {
  inline membuf(int _size, char _char[]) {
    this->setg(_char, _char, _char + _size);
  }
};

class Parser {
  Parser(std::istream)

};

} // namespace http

#endif // !PARSER
