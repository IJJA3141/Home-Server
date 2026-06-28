#include "../common/logger.hpp"
#include "reverse_proxy.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/tcp.h>
#include <span>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <system_error>
#include <unistd.h>

TcpServer::Client::Client(int listening_socket, int epoll, const protocol::Handler<ForwardPolicy>& handler)
    : socket{-1}, epoll{-1}, handler{handler}
{
  auto log = Logger::get("TCP Client", [] { return strerror(errno); });

  sockaddr_in addr{AF_INET};
  socklen_t len = sizeof addr;
  this->socket = accept4(listening_socket, reinterpret_cast<sockaddr*>(&addr), &len, SOCK_NONBLOCK);
  if (this->socket < 0)
  {
    log.error("socket accept failed");
    throw std::system_error(errno, std::system_category(), "accept");
  }
  log.debug("connection accepted");

  epoll_event client_event(EPOLLIN | EPOLLET | EPOLLRDHUP, epoll_data{.ptr = this});
  if (epoll_ctl(epoll, EPOLL_CTL_ADD, this->socket, &client_event))
  {
    if (close(this->socket)) log.warn("socket close failed");
    log.error("epoll (fd={}) registration failed", epoll);
    throw std::system_error(errno, std::system_category(), "epoll ctl");
  }
  log.debug("epoll registered");

  if (!inet_ntop(AF_INET, &addr, this->ip, sizeof addr)) log.error("failed to translate IP");
  this->port = ntohs(addr.sin_port);
  this->epoll = epoll;

  log.info("new client (socket={}, ip={}) connected to server (socket={}, epoll={})", socket, ip, listening_socket,
           epoll);
}

TcpServer::Client::~Client()
{
  const auto& log = Logger::get(std::format("TCP Client {}", this->ip), [] { return strerror(errno); });
  if (this->socket >= 0 && close(this->socket)) log.warn("socket close failed");
  if (this->epoll >= 0 && epoll_ctl(this->epoll, EPOLL_CTL_DEL, this->socket, nullptr))
    log.warn("epoll close failed");
  log.info("disconnected...");
}

void TcpServer::Client::client_event(uint32_t events)
{
  auto log = Logger::get(std::format("TCP Client {}", this->ip));

  bool peer_closed = events & (EPOLLHUP | EPOLLRDHUP);
  bool can_recv = events & EPOLLIN;
  bool can_send = events & EPOLLOUT;

  if (peer_closed)
  {
    log.info("disconnected...", this->ip);
    delete this;
    return;
  }

  // read all it can to connection_buffer
  if (can_recv)
  {
    std::span<std::byte> buffer;
    ssize_t bytes;

    while ((buffer = this->connection_buffer.write_buffer()).size() > 0 &&
           (bytes = this->recv(this->socket, buffer.data(), buffer.size()) > 0))
      this->connection_buffer.acknowledge(bytes);
  }

  // either parse header
  // or route to hosts
  // or bad host
  if(this->a.state == A::State::h)
  {
  }

  //
  // if (this->manager.wants_handler()) this->handler(this->manager.request, this->manager.response);
  //
  // if (can_send)
  // {
  //   std::span<const std::byte> buffer;
  //   ssize_t bytes;
  //
  //   while ((buffer = this->manager.send_buffer()).size() > 0 &&
  //          (bytes = this->send(this->socket, buffer.data(), buffer.size(), 0) > 0))
  //     this->manager.discard(bytes);
  //
  //   if (this->manager.release_buffer()) this->connection_buffer.clear();
  // }

  // if (this->wants_disconnect())
  {
    log.info("disconnecting...", this->ip);
    delete this;
    return;
  }
}

ssize_t TcpServer::Client::recv(int __fd, void* __buf, size_t __nbytes)
{
  auto log = Logger::get(std::format("TCP Client {}", this->ip), [] { return strerror(errno); });
  ssize_t bytes = ::read(__fd, __buf, __nbytes);

  if (bytes < 0)
  {
    switch (errno)
    {
    // recoverable
    case EAGAIN:
#if EAGAIN != EWOULDBLOCK
    case EWOULDBLOCK:
#endif
      log.warn("recoverable erorr received");
      break;

    case EINVAL:
      log.warn("irrecoverable error");
      this->connection_failed = true;
      break;

    // irrecoverable
    case EBADF:
    case ECONNREFUSED:
    case EFAULT:
    case EINTR:
    case ENOMEM:
    case ENOTCONN:
    case ENOTSOCK:
    default:
      log.error("irrecoverable error due to invalid argument(s)");
      throw std::system_error(errno, std::system_category(), "recv");
    }
  }

  return bytes;
}

ssize_t TcpServer::Client::send(int __fd, const void* __buf, size_t __n, int __flags)
{
  auto log = Logger::get(std::format("TCP Client {}", this->ip), [] { return strerror(errno); });
  ssize_t bytes = ::send(__fd, __buf, __n, __flags);
  if (bytes < 0)
  {
    switch (errno)
    {
    // recoverable
    case EAGAIN:
    case ENOBUFS:
#if EAGAIN != EWOULDBLOCK
    case EWOULDBLOCK:
#endif
      log.warn("recoverable error sent");
      break;

      // failed
    case EPIPE:
    case EINTR:
    case ECONNRESET:
      log.warn("irrecoverable error");
      this->connection_failed = true;
      break;

      // irrecoverable
    case EACCES:
    case EALREADY:
    case EBADF:
    case EDESTADDRREQ:
    case EFAULT:
    case EINVAL:
    case EISCONN:
    case EMSGSIZE:
    case ENOMEM:
    case ENOTCONN:
    case ENOTSOCK:
    case EOPNOTSUPP:
    default:
      log.error("irrecoverable error due to invalid argument(s)");
      throw std::system_error(errno, std::system_category(), "send");
    }
  }

  return bytes;
}
