#include "server.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

TcpServer::Client::Client(int _listening_socket, const ForwardingTable& _forwarding_table)
    : forwarding_table_{_forwarding_table}, uuid_(Uuid::generate()), addr_{AF_INET}
{
  auto log = Logger::get("TCP Client", [] { return strerror(errno); });
  log.debug("connecting...");

  socklen_t len = sizeof addr_;
  this->socket = accept4(_listening_socket, reinterpret_cast<sockaddr*>(&addr_), &len, SOCK_NONBLOCK);
  if (this->socket < 0)
  {
    log.error("socket accept failed");
    throw std::runtime_error("socket accept failed");
  }

  this->ip_.resize(INET_ADDRSTRLEN);
  inet_ntop(AF_INET, &addr_, this->ip_.data(), sizeof addr_);
}

TcpServer::Client::~Client()
{
  const auto& log = Logger::get(std::format("TCP Client IP={}", this->ip_), [] { return strerror(errno); });

  if (this->socket >= 0 && close(this->socket)) log.error("socket close failed");
  log.info("disconnected...");
}

void TcpServer::Client::notify_read() {};
