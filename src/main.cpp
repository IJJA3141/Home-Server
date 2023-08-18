#include "./tcp-server/tcp-server.hpp"
#include "./tls-server/tls-server.hpp"

#include <iostream>

int main(int argc, char *argv[]) {

  http::TlsServer server = http::TlsServer("./ssl/cert.pem", "./ssl/key.pem");
  server.listen("0.0.0.0", 5000);

  std::cout << server.state << std::endl;

  return 0;
}
