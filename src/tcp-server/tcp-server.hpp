#ifndef TCP_SERVER
#define TCP_SERVER

#include "../client/client.hpp"
#include "../macro.hpp"
#include "../parser/parser.hpp"

#include <arpa/inet.h>
#include <array>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace http {

class TcpServer {
public:
  std::string name;
  std::vector<std::thread *> vThread;

  TcpServer(const char *_pName = nullptr);
  void Bind(const char *_pPort);
  void Listen();
  void Connect(http::Client *_pClient);
  void Add(http::Method _method, const char *_pPath,
           std::function<void(void *_pVoid)> _λ);
  ~TcpServer();

protected:
  std::array<std::map<std::string, std::function<void(void *_pVoid)>>, 9> _vMap;
  static int serverCount_;

  int socket_;
  struct sockaddr_in hint_;
  int port_;

  virtual Client *GetClient_(const int *_pSocket) {
    return new http::Client(_pSocket);
  }
};
} // namespace http

#endif // !TCP_SERVER
