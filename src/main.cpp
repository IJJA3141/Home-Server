#include "./tcp-server/tcp-server.hpp"
#include "./tls-server/tls-server.hpp"

#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]) {

  std::cout << argc << std::endl;

  const char *ip = "192.168.1.123";
  const char *port = "60";

  if (argc == 3) {
    ip = argv[1];
    port = argv[2];
  };

  http::TlsServer server = http::TlsServer("./ssl/cert.pem", "./ssl/key.pem");
  server.listen(ip, atoi(port));

  return 0;
}
