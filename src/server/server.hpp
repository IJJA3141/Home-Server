#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <vector>

namespace http
{

class Client
{
public:
  int read();
};
class SSLClient;

class TcpServer
{
public:
  std::vector<std::thread *> threads;

  TcpServer();

  void connect(http::Client *_client);
  void bind(const int _port);
  void listen();

protected:
  int socket_;
  struct sockaddr_in hint_;
  int port_;

  virtual Client *getClient_(const int *_socket);
};

}; // namespace http
