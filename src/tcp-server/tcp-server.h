#ifndef TCPSERVER
#define TCPSERVER

#include <string>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>

namespace http {
class TcpServer {
  // public members
public:
  // private members
private:
  int socket_;
  std::string ip_address_;
  int port_;
  int new_socket_;
  long incomingMessage_;
  struct sockaddr_in socketAddress_;
  unsigned int socketAddress_len_;
  std::string serverMessage_;

  // public methods
public:
  TcpServer(std::string _id_address, int _port);
  ~TcpServer();

  // private methods
private:
};

}; // namespace http

#endif // !TCPSERVER
