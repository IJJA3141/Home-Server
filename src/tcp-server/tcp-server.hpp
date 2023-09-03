#ifndef TCP_SERVER
#define TCP_SERVER

#include "../client/client.hpp"
#include "../parser/parser.hpp"

#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
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
  void Connect(http::Client _client);
  void add(http::Method _method, const char *_path,
           std::function<void(void *_pVoid)> _λ);

protected:
  std::array<std::map<std::string, std::function<void(void *_pVoid)>>, 9> _vMap;
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
