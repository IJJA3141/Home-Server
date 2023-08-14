#include <iostream>

#include "./parser/parser.h"
#include "./request/request.h"

int main(int argc, char *argv[]) {

  http::parse("GET /test/id=4324hkj24h32hkldshjklfdsalhl&q=jfds+jkds HTTP/1.1\nHost: 192.168.1.123:53509\n", 0);

  return 0;
}
