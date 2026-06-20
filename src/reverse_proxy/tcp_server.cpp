#include "../config.hpp"
#include "../logger/logger.hpp"
#include "reverse_proxy.hpp"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <utility>

TcpServer::TcpServer(const std::string& _ip, uint16_t _port, const Handler _request)
    : ip_(_ip), port_(_port), listening_socket_(-1), epoll_fd_(-1), request_handler_(_request)
{
  const auto& log = Logger::get("TCP Server", [] { return strerror(errno); });

  log.debug("creating and binding socket");

  sockaddr_in addr{AF_INET, htons(_port)};
  if (_ip.empty()) addr.sin_addr = in_addr(htonl(INADDR_ANY));
  else if (inet_pton(AF_INET, _ip.c_str(), &addr.sin_addr) != 1)
  {
    log.crit("IP resolution failed (IP={})", ip_);
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

  log.debug("socket bind ({}:{})\ncreating epoll", _ip.empty() ? "0.0.0.0" : _ip, _port);

  this->epoll_fd_ = epoll_create1(0);
  if (this->epoll_fd_ < 0)
  {
    close(this->listening_socket_);

    log.crit("epoll creation failed (socket={})", this->listening_socket_);
    throw std::runtime_error("epoll creation failed");
  }

  epoll_event listening_socket_event(EPOLLIN | EPOLLRDHUP, epoll_data{.fd = listening_socket_});
  if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, this->listening_socket_, &listening_socket_event))
  {
    close(this->listening_socket_);
    close(this->epoll_fd_);

    log.crit("epoll_ctl failed (socket={})", this->listening_socket_);
    throw std::runtime_error("epoll_ctl failed");
  }

  log.debug("eppoll created (fd={})", this->epoll_fd_);
  log.info("creation successful, ready to listen");
}

TcpServer::~TcpServer()
{
  const auto& log = Logger::get(std::format("TCP Client (IP={})", this->ip_), [] { return strerror(errno); });
  if (this->listening_socket_ >= 0 && close(this->listening_socket_)) log.error("socket close failed");
  if (this->epoll_fd_ >= 0) close(this->epoll_fd_);
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

  log.info("socket listening ({}:{})", this->ip_.empty() ? "0.0.0.0" : this->ip_, this->port_);

  epoll_event events[REVERSE_PROXY_EPOLL_SIZE];
  this->running_ = true;

  // main loop
  while (this->running_)
  {
    int n = epoll_wait(this->epoll_fd_, events, REVERSE_PROXY_EPOLL_SIZE, -1);
    if (n < 0)
    {
      log.crit("epoll wait failed");
      throw std::runtime_error("epoll wait failed");
    }

    log.debug("epoll received {} events", n);

    for (size_t i = 0; i < n; ++i)
    {
      bool closed = events[i].events & (EPOLLHUP | EPOLLRDHUP);

      if (events[i].data.fd == this->listening_socket_)
      { // event from serv.
        if (closed) log.info("closing...");
        else this->accept();
      }
      else
      { // event form a client
        Client* client = static_cast<Client*>(events[i].data.ptr);
        if (closed || client->notify()) delete client;
      }
    }
  }
}

void TcpServer::accept()
{
  try
  {
    new Client(this->listening_socket_, this->epoll_fd_, this->request_handler_);
  }
  catch (const std::runtime_error& e)
  {
  }
}

TcpServer::Client::Client(const int _listening_socket, const int _epoll, const Handler _request_handler)
    : socket(-1), epoll(-1), request_handler(_request_handler)
{
  auto log = Logger::get("TCP Client", [] { return strerror(errno); });
  log.debug("connecting...");

  sockaddr_in addr{AF_INET};
  socklen_t len = sizeof addr;
  this->socket = accept4(_listening_socket, reinterpret_cast<sockaddr*>(&addr), &len, SOCK_NONBLOCK);
  if (this->socket < 0)
  {
    log.error("socket accept failed");
    throw std::runtime_error("socket accept failed");
  }

  log.debug("registering (socket={}) to epoll (fd={})", this->socket, _epoll);
  epoll_event client_event(EPOLLIN | EPOLLET | EPOLLRDHUP, epoll_data{.ptr = this});
  if (epoll_ctl(_epoll, EPOLL_CTL_ADD, this->socket, &client_event))
  {
    close(this->socket);

    log.error("epoll (fd={}) registration failed", _epoll);
    throw std::runtime_error("epoll registration failed");
  }

  inet_ntop(AF_INET, &addr, this->ip, sizeof addr);
  this->port = ntohs(addr.sin_port);
  this->epoll = _epoll;

  log.info("new client (socket={}, ip={}) connected to server (socket={}, epoll={})", this->socket, this->ip,
           _listening_socket, _epoll);
}

TcpServer::Client::~Client()
{
  const auto& log = Logger::get(std::format("TCP Client IP={}", this->ip), [] { return strerror(errno); });

  if (this->socket >= 0 && close(this->socket)) log.error("socket close failed");
  if (this->epoll >= 0) epoll_ctl(this->epoll, EPOLL_CTL_DEL, this->socket, nullptr);
  log.info("disconnected...");
}

bool TcpServer::Client::notify()
{
  ssize_t bytes;

  auto log = Logger::get(std::format("TCP Client (IP={})", this->ip), [] { return strerror(errno); });

  { // one func
    // should recv into buffer, hanlde ssl error and disconnection
    bytes = this->recv(this->socket, this->connection_buffer.write(), this->connection_buffer.capacity(), 0);
    if (bytes < 0)
    {
      log.error("recv failed");
      return false;
    }
    else if (bytes == 0) return true;
    else this->connection_buffer.acknowledge(bytes);

    log.debug("received {} bytes", bytes);

    bytes = protocol::HTTP::Request::parse(this->connection_buffer.read(), this->parser_ctx);
    this->connection_buffer.discard(bytes);
  }

  std::string response;
  switch (this->parser_ctx.result)
  {
  case protocol::ParserResult::Invalid:
    log.warn("response parsing failed");

    // TODO change this at some point
    response = protocol::HTTP::standard_response(400);
    this->send(this->socket, response.c_str(), response.size(), 0);
    return true;

  case protocol::ParserResult::NeedMoreData:
    log.debug("need more data");
    return false;

  case protocol::ParserResult::Complete:
    protocol::HTTP::Request request = this->parser_ctx.construct();
    log.debug("received\n{}", std::string(request));
    request.headers[HTTP_CONNECTION_TYPE] = this->connection_type();
    request.headers[HTTP_CLIENT_IP] = this->ip;

    response = this->request_handler(request); // TODO
    bytes = this->send(this->socket, response.c_str(), response.size(), 0);
    if (bytes < 0)
    {
      log.error("send failed");
      return true;
    }
    log.debug("{} bytes send", bytes);

    // 🫪
    if (request.version == protocol::HTTP::Version::HTTP_10)
      return !(request.headers.contains("connection") && request.headers["connection"] == "keep-alive");

    if (request.version == protocol::HTTP::Version::HTTP_11)
      return request.headers.contains("connection") && request.headers["connection"] == "close";

    return true;
  }

  std::unreachable();
}

ssize_t TcpServer::Client::recv(int __fd, void* __buf, size_t __n, int __flags)
{
  return ::recv(__fd, __buf, __n, __flags);
}

ssize_t TcpServer::Client::send(int __fd, const void* __buf, size_t __n, int __flags)
{
  return ::send(__fd, __buf, __n, __flags);
}
