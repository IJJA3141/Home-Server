#ifndef PARSER
#define PARSER

#include <cstddef>
#include <string>

namespace http {

enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE, PATCH, NONE };

struct Request {
  Method method;
  const char *path;
  bool upgrade;
  size_t contentSize;
  const char *body;

  Request(char *_message);
};
} // namespace http

#endif // !PARSER
