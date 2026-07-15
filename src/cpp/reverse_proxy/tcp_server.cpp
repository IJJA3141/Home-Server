#include "server.hpp"
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <system_error>

TcpServer::TcpServer(const std::string& _ip, uint16_t _port, Epoll _epoll, const Table& _forwarding)
    : epoll_{_epoll}, forwarding_{_forwarding}
{
  const auto& log = Logger::get("TCP Server", [] { return strerror(errno); });

  log.debug("creating and binding socket");

  sockaddr_in addr{AF_INET, htons(_port)};
  if (_ip.empty()) addr.sin_addr = in_addr(htonl(INADDR_ANY));
  else if (inet_pton(AF_INET, _ip.c_str(), &addr.sin_addr) != 1)
  {
    log.crit("IP resolution failed (IP={})", _ip);
    throw std::runtime_error("IP resolution failed");
  }

  int opts[] = {TCP_NODELAY, TCP_COOKIE_TRANSACTIONS};
  this->listening_socket_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (this->listening_socket_ < 0)
  {
    log.crit("socket creation failed (fd={})", this->listening_socket_);
    throw std::runtime_error("socket creation failed");
  }

  if (setsockopt(this->listening_socket_, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof opts))
  {
    close(this->listening_socket_);

    log.crit("socket option configuration failed");
    throw std::runtime_error("socket option configuration failed");
  }

  log.debug("socket created (fd={})", this->listening_socket_);

  if (bind(this->listening_socket_, reinterpret_cast<sockaddr*>(&addr), sizeof addr))
  {
    close(this->listening_socket_);

    log.crit("socket binding failed (fd={})", this->listening_socket_);
    throw std::runtime_error("socket binding failed");
  }

  log.debug("socket bind ({}:{})\nadding epoll", _ip.empty() ? "0.0.0.0" : _ip, _port);
  _epoll.add<EPOLLIN | EPOLLRDHUP | EPOLLET>(*this);

  log.info("creation successful, ready to listen");
}

TcpServer::~TcpServer()
{
  const auto& log = Logger::get(std::format("TCP Server"), [] { return strerror(errno); });
  if (this->listening_socket_ >= 0 && close(this->listening_socket_)) log.error("socket close failed");

  try
  {
    this->epoll_.del(*this);
  }
  catch (const std::system_error& e)
  {
    log.error("epoll threw an error while deregistering error: {}", e.what());
  }

  for (auto [_, client] : this->clients_)
    try
    {
      delete client;
    }
    catch (const std::system_error& e)
    {
      log.error("client threw an error while deregistering error: {}", e.what());
    }

  log.info("closed...");
}

void TcpServer::listen()
{
  const auto& log =
      Logger::get(std::format("TCP Server fd={}", this->listening_socket_), [] { return strerror(errno); });
  log.debug("marking socket for listening");

  if (::listen(this->listening_socket_, REVERSE_PROXY_MAX_QUEUE))
  {
    log.crit("socket listening failed");
    throw std::runtime_error("socket listening failed");
  }

  log.info("socket listening");
}

void TcpServer::drop_child(Uuid _child_uuid)
{
  auto node = this->clients_.extract(_child_uuid);
  if (!node.empty()) try
    {
      delete node.mapped();
    }
    catch (const std::system_error& e)
    {
      const auto& log = Logger::get(std::format("TCP Server"), [] { return strerror(errno); });
      log.error("client threw an error while deregistering error: {}", e.what());
    }
}

void TcpServer::notify_read()
{
  try
  {
    auto client = new TcpServer::Client(this->listening_socket_, *this);
    this->clients_.emplace(client->uuid(), client);
  }
  catch (const std::runtime_error& e)
  {
    const auto& log = Logger::get(std::format("TCP Server"), [] { return strerror(errno); });
    log.error("client threw an error while registering error: {}", e.what());
  }
}

// place holders
void TcpServer::notify_half_close() { Logger::debug("server half closed"); }
void TcpServer::notify_close() { Logger::debug("server closed"); }
void TcpServer::notify_error() { Logger::debug("server error"); }
