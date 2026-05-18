#include "../logger/logger.hpp"
#include "reverse_proxy.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <sys/socket.h>
#include <utility>

TcpServer::TcpServer(const std::string& _ip, uint16_t _port, const Handler _request)
    : ip_(_ip), port_(_port), listening_socket_(-1), epoll_fd_(-1), request_handler_(_request)
{
  auto log = Logger::New();

  sockaddr_in addr{AF_INET, htons(_port)};
  if (_ip.empty()) addr.sin_addr = in_addr(htonl(INADDR_ANY));
  else if (inet_pton(AF_INET, _ip.c_str(), &addr.sin_addr) != 1)
  {
    log.crit("resolving IP failed: {}", strerror(errno));
    throw std::runtime_error(std::format("resolving IP failed: {}", strerror(errno)));
  }

  int opts[] = {TCP_NODELAY, TCP_COOKIE_TRANSACTIONS};
  this->listening_socket_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  if (this->listening_socket_ < 0)
  {
    log.crit("socket creation failed: {}", strerror(errno));
    throw "SocketCreationException";
  }
  log.info("socket created (fd={})", this->listening_socket_);

  if (setsockopt(this->listening_socket_, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof opts))
  {
    close(this->listening_socket_);

    log.crit("socket option configuration failed: {}", std::strerror(errno));
    throw "SocketOptionsException";
  }

  if (bind(this->listening_socket_, reinterpret_cast<sockaddr*>(&addr), sizeof addr))
  {
    close(this->listening_socket_);

    log.crit("socket bind failed: {}", strerror(errno));
    throw "BindingException";
  }
  log.info("socket bind ({}:{})", _ip.empty() ? "0.0.0.0" : _ip, _port);

  this->epoll_fd_ = epoll_create1(0);
  if (this->epoll_fd_ < 0)
  {
    close(this->listening_socket_);

    log.crit("epoll creation failed: {}", strerror(errno));
    throw "EpollCreationException";
  }

  epoll_event listening_socket_event(EPOLLIN | EPOLLRDHUP, epoll_data{.fd = listening_socket_});
  if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, this->listening_socket_, &listening_socket_event))
  {
    close(this->listening_socket_);
    close(this->epoll_fd_);

    log.crit("epoll creation failed: {}", strerror(errno));
    throw "EpollCtlException";
  }

  log.info("eppoll created (fd={})", this->epoll_fd_);
}

void TcpServer::listen()
{
  auto log = Logger::New();

  if (::listen(this->listening_socket_, 100))
  {
    log.crit("socket filed to listen {}", strerror(errno));
    throw "";
  }
  log.info("socket listen (addr={}:{})", this->ip_.empty() ? "0.0.0.0" : this->ip_, this->port_);

  epoll_event events[REVERSE_PROXY_EPOLL_SIZE];
  this->running_ = true;

  while (this->running_)
  {
    int n = epoll_wait(this->epoll_fd_, events, REVERSE_PROXY_EPOLL_SIZE, -1);
    if (n < 0)
    {
      log.crit("{}", strerror(errno));
      throw "";
    }

    for (size_t i = 0; i < n; ++i)
    {
      bool closed = events[i].events & (EPOLLHUP | EPOLLRDHUP);

      if (events[i].data.fd == this->listening_socket_)
      { // event from serv.
        if (closed) log.info("socket closed");
        else accept();
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
  catch (const std::exception& e)
  {
  }
}

TcpServer::Client::Client(const int _listening_socket, const int _epoll, const Handler _request_handler)
    : socket(-1), epoll(-1), request_handler(_request_handler)
{
  auto log = Logger::New();

  sockaddr_in addr{AF_INET};
  socklen_t len = sizeof addr;
  this->socket = accept4(_listening_socket, reinterpret_cast<sockaddr*>(&addr), &len, SOCK_NONBLOCK);
  if (this->socket < 0)
  {
    log.error("failed to accept {}", strerror(errno));
    throw "";
  }

  epoll_event client_event(EPOLLIN | EPOLLET | EPOLLRDHUP, epoll_data{.ptr = this});
  if (epoll_ctl(_epoll, EPOLL_CTL_ADD, this->socket, &client_event))
  {
    close(this->socket);

    log.crit("epoll creation failed: {}", strerror(errno));
    throw "EpollCtlException";
  }

  inet_ntop(AF_INET, &addr, this->ip, sizeof addr);
  this->port = ntohs(addr.sin_port);
  this->epoll = _epoll;
}

TcpServer::Client::~Client()
{
  auto log = Logger::New();

  if (this->socket >= 0)
  {
    if (close(this->socket)) log.error("closed failed {}", strerror(errno));
    else log.info("client socket closed");
  }
  if (this->epoll >= 0) epoll_ctl(this->epoll, EPOLL_CTL_DEL, this->socket, nullptr);
}

bool TcpServer::Client::notify()
{
  ssize_t bytes;

  auto log = Logger::New();

  bytes = this->recv(this->socket, this->connection_buffer.write(), this->connection_buffer.capacity(), 0);
  if (bytes < 0)
  {
    log.error("recv failed {}", strerror(errno));
    return true;
  }
  if (bytes == 0) return true;

  this->connection_buffer.acknowledge(bytes);

  bytes = protocol::HTTP::Request::parse(this->connection_buffer.read(), this->parser_ctx);
  this->connection_buffer.discard(bytes);

  std::string response;
  switch (this->parser_ctx.result)
  {
  case protocol::ParserResult::Invalid:
    response = protocol::HTTP::standard_response(400);
    this->send(this->socket, response.c_str(), response.size(), 0);
    return true;

  case protocol::ParserResult::NeedMoreData:
    return false;

  case protocol::ParserResult::Complete:
    protocol::HTTP::Request request = this->parser_ctx.construct();
    request.headers["x-connection-type"] = this->connection_type();
    request.headers["x-client-id"] = this->ip;

    response = this->request_handler(request); // TODO
    bytes = this->send(this->socket, response.c_str(), response.size(), 0);
    if (bytes < 0)
    {
      log.error("send failed {}", strerror(errno));
      return true;
    }

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
