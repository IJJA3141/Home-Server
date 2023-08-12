#ifndef PARSER
#define PARSER

#include "../request/request.h"

#include <cstddef>
namespace http {

http::Request parse(const char *_buffer, size_t _size);

}

#endif // !PARSER
