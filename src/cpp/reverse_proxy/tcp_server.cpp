#include "../common/logger.hpp"
#include "../config.hpp"
#include "reverse_proxy.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <netinet/tcp.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <system_error>
#include <unistd.h>

TcpServer::TcpServer(const std::string& ip, uint16_t port, protocol::Handler<ForwardPolicy> handler)
    : listening_socket_{-1}, epoll_fd_{-1}, handler_{handler}
{
  const auto& log = Logger::get("TCP Server", [] { return strerror(errno); });

  // ip
  sockaddr_in addr{AF_INET, htons(port)};
  if (ip.empty()) addr.sin_addr = in_addr(htonl(INADDR_ANY));
  else if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1)
  {
    log.crit("IP resolution failed (IP={})", ip);
    throw std::invalid_argument("Invalid IPv4 " + ip);
  }

  // socket
  this->listening_socket_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (this->listening_socket_ < 0)
  {
    log.crit("socket creation failed (fd={})", this->listening_socket_);
    throw std::system_error(errno, std::system_category(), "socket");
  }

  int opts[] = {TCP_NODELAY, TCP_COOKIE_TRANSACTIONS};
  if (setsockopt(this->listening_socket_, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof opts))
  {
    if (close(this->listening_socket_)) log.error("socket close failed");
    log.crit("socket option configuration failed");
    throw std::system_error(errno, std::system_category(), "socket opt");
  }
  log.debug("socket created (fd={})", this->listening_socket_);

  // bind
  if (bind(this->listening_socket_, reinterpret_cast<sockaddr*>(&addr), sizeof addr))
  {
    if (close(this->listening_socket_)) log.error("socket close failed");
    log.crit("socket binding failed (fd={})", this->listening_socket_);
    throw std::system_error(errno, std::system_category(), "bind");
  }
  log.debug("socket bind ({}:{})\ncreating epoll", ip.empty() ? "0.0.0.0" : ip, port);

  // epoll
  this->epoll_fd_ = epoll_create1(0);
  if (this->epoll_fd_ < 0)
  {
    if (close(this->listening_socket_)) log.error("socket close failed");
    log.crit("epoll creation failed (fd={})", this->listening_socket_);
    throw std::system_error(errno, std::system_category(), "epoll");
  }

  epoll_event listening_socket_event(EPOLLIN | EPOLLRDHUP, epoll_data{.fd = listening_socket_});
  if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, this->listening_socket_, &listening_socket_event))
  {
    if (close(this->listening_socket_)) log.error("socket close failed");
    if (close(this->epoll_fd_)) log.error("epoll close failed");
    log.crit("epoll_ctl failed (socket={}, epoll={})", this->listening_socket_, this->epoll_fd_);
    throw std::system_error(errno, std::system_category(), "epoll ctl");
  }
  log.debug("eppoll created (fd={})", this->epoll_fd_);

  log.info("created...");
}

TcpServer::~TcpServer()
{
  auto log = Logger::get("TCP Server");
  if (this->listening_socket_ >= 0 && close(this->listening_socket_)) log.error("socket close failed");
  if (this->epoll_fd_ >= 0 && close(this->epoll_fd_)) log.error("epoll close failed");
  log.info("deleted...");
}

void TcpServer::listen()
{
  auto log = Logger::get("TCP Server", [] { return strerror(errno); });

  if (::listen(this->listening_socket_, REVERSE_PROXY_MAX_QUEUE))
  {
    log.crit("socket listening failed");
    throw std::system_error(errno, std::system_category(), "listen");
  }

  log.info("listening...");

  epoll_event events[REVERSE_PROXY_EPOLL_SIZE];
  this->running_ = true;

  // main loop
  while (this->running_)
  {
    int n = epoll_wait(this->epoll_fd_, events, REVERSE_PROXY_EPOLL_SIZE, -1);
    if (n < 0)
    {
      log.crit("epoll wait failed");
      throw std::system_error(errno, std::system_category(), "epoll wait");
    }

    log.debug("received {} events", n);
    for (size_t i = 0; i < n; ++i)
    {
      epoll_event event = events[i];
      if (event.data.fd == this->listening_socket_) this->server_event(event.events);
      else static_cast<Client*>(event.data.ptr)->client_event(event.events);
    }
  }
}

void TcpServer::server_event(uint32_t events)
{
  if (events & (EPOLLHUP | EPOLLRDHUP))
  {
    this->running_ = false;
  }
  else
  {
    this->accept();
  }
}

void TcpServer::accept()
{
  auto log = Logger::get("TCP Server");

  log.info("accepting new client connection");
  try
  {
    new TcpServer::Client(this->listening_socket_, this->epoll_fd_, this->handler_);
  }
  catch (const std::system_error& e)
  { // client connection failed
    log.warn("client connection failed (error={})", e.what());
  }
}
