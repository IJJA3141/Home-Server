#ifndef TCP_SERVER
#define TCP_SERVER

#include "../client/client.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace http {

class TcpServer {
public:
  std::vector<std::thread *> vThread;
  std::string name;

  TcpServer(const char *_name = nullptr);
  void bind(const char *_port);
  void listen();
  static void Connect(http::Client _client, const char *_name);

protected:
  static int serverCount;

  int socket_;
  struct sockaddr_in hint_;
  int port_;

  inline virtual Client GetClient_(const int *_pSocket) {
    return http::Client(_pSocket);
  }
};
} // namespace http

#endif // !TCP_SERVER
