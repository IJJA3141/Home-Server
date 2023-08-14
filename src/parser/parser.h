#ifndef PARSER
#define PARSER

#include "../request/request.h"

#include <cstddef>
namespace http {

http::Request parse(char *_pBuffer, size_t _size);

}

#endif // !PARSER
