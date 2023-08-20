#ifndef TCP_SERVER
#define TCP_SERVER

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace http {

struct Client {
  Client(const int *_pSocket);

  sockaddr_in client;
  socklen_t size;
  int socket;
  char buffer[4096];
};

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
};
} // namespace http

#endif // !TCP_SERVER
