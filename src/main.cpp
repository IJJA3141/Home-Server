#include "./tcp-server/tcp-server.h"

int main(int argc, char *argv[]) {
  http::TcpServer server = http::TcpServer("0.0.0.0", 8080);
  return 0;
}
