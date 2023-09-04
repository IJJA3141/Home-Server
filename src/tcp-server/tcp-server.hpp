#ifndef TCP_SERVER
#define TCP_SERVER

#include "../parser/parser.hpp"
#include "../client/client.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace http {

class TcpServer {
public:
  TcpServer();
  void bind(char *_port);
  void listen();

protected:
  int socket_;
  struct sockaddr_in hint_;
  int port_;

  virtual void ConnectionHandler_();
  inline virtual Client GetClient_(const int *_pSocket) {
    return http::Client(_pSocket);
  }
};
} // namespace http

#endif // !TCP_SERVER
