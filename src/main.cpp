#include "server/server.hpp"

int main (int _argc, char *_argv[]) {

  TcpServer serv;
  serv.bind(80);
  serv.listen();
  
  return 0;
}
