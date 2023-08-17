#include "./tls-server/tls-server.hpp"

#include <iostream>

int main(int argc, char *argv[]) {

  http::TlsServer server = http::TlsServer("", "");
  server.listen("127.0.0.0", 5018);

  std::cout << server.state << std::endl;

  return 0;
}
