#ifndef TCPSERVER
#define TCPSERVER

#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace http {
class TcpServer {
public: // Public members
  static const std::string test;

private: // Private members
  // Singleton
  static TcpServer *pTcpServer_;

  // Server
  int socket_;
  struct sockaddr_in hint_;
  int port_;

  // Client
  sockaddr_in client_;
  socklen_t clientSize_;
  int clientSocket_;
  char clientBuffer[4096];

public: // Public methods
  static TcpServer *get();
  int init(std::string _ip, int _port);
  int listen();

private: // Private methods
  TcpServer();

public: // Public operator
  void operator=(const TcpServer &) = delete;

private: // Private operator
};
} // Namespace http

const std::string http::TcpServer::test = "test";

#endif // !TCPSERVER
