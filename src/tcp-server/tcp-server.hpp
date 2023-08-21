#ifndef TCP_SERVER
#define TCP_SERVER

#include "../client/client.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace http {

class TcpServer {
public:
  std::vector<std::thread *> vThread;

  TcpServer();
  void bind(char *_port);
  void listen();
  static void Connect(http::Client _client);

protected:
  int socket_;
  struct sockaddr_in hint_;
  int port_;

  inline virtual Client GetClient_(const int *_pSocket) {
    return http::Client(_pSocket);
  }
};
} // namespace http

#endif // !TCP_SERVER
