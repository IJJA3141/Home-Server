#ifndef TCP_SERVER
#define TCP_SERVER

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

namespace http {

struct Client {
  Client(int* _pSocket);

  sockaddr_in client;
  socklen_t size;
  int socket;
  char buffer[4096];
};

class TcpServer {
private:
  enum State {
    running,
    initializing,
    initialized,
    binding,
    listening,
    brocken
  };

public:
  State state;

  TcpServer();
  void listen(std::string _address, int _port);

protected:
  int socket_;
  struct sockaddr_in hint_;
  int port_;

  virtual void ConnectionHandler_();
};

} // namespace http

#endif // !TCP_SERVER
