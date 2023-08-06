#include <iostream>

#include "./tcp-server/tcp-server.h"

int main(int argc, char *argv[]) {
  std::cout << http::TcpServer::test << std::endl;
  /*
    http::TcpServer *pServer = http::TcpServer::get();

    int initValue = pServer->init("0.0.0.0", 50000);
    if (initValue != 0)
      return initValue;

    return pServer->listen();
  */

  return 0;
}
