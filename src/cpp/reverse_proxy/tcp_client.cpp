#include "../common/utils.hpp"
#include "server.hpp"

#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <system_error>

TcpServer::Client::Client(int _listening_socket, TcpServer& _parent)
    : parent{_parent}, uuid_(Uuid::generate()), addr_{AF_INET}
{
  auto log = Logger::get("TCP Client", [] { return strerror(errno); });
  log.debug("connecting...");

  socklen_t len = sizeof addr_;
  this->socket_ = accept4(_listening_socket, reinterpret_cast<sockaddr*>(&addr_), &len, SOCK_NONBLOCK);
  if (this->socket_ < 0)
  {
    log.error("socket accept failed");
    throw std::runtime_error("socket accept failed");
  }

  this->ip_.resize(INET_ADDRSTRLEN);
  inet_ntop(AF_INET, &addr_, this->ip_.data(), sizeof addr_);

  this->parent.epoll_.add<EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET>(*this);
}

TcpServer::Client::~Client()
{
  const auto& log = Logger::get(std::format("TCP Client IP={}", this->ip_), [] { return strerror(errno); });

  if (this->socket_ >= 0 && close(this->socket_)) log.error("socket close failed");
  this->parent.epoll_.del(*this);
  log.info("disconnected...");
}

bool TcpServer::Client::recv()
{
  auto in = this->in_.write_buffer();
  ssize_t bytes = ::recv(this->socket_, in.data(), in.size(), 0);
  if (bytes >= 0)
  {
    this->in_.acknowledge(bytes);
    return true;
  }

  switch (errno)
  {
#if EAGAIN != EWOULDBLOCK
  case EWOULDBLOCK:
#endif
  case EAGAIN:
    return false;

  case ENOTCONN:
  case ECONNRESET:
  case ETIMEDOUT:
    this->notify_half_close();
    return false;

  case EBADF:
  case EINTR:
  case ENOTSOCK:
  case EINVAL:
  case EOPNOTSUPP:
    throw std::system_error(errno, std::system_category(), "recv failed");

  case EIO:
  case ENOBUFS:
  case ENOMEM:
    throw std::system_error(errno, std::system_category(), "recv might have failed");
  }

  std::unreachable();
}

void TcpServer::Client::send(std::span<const std::byte> _bytes)
{
  if (this->out_.empty())
  { // send directly
    ssize_t bytes = ::send(this->socket_, _bytes.data(), _bytes.size(), 0);
    if (bytes >= 0)
    {
      if (bytes >= _bytes.size()) return;
    }
  }
  else if (_bytes.size() <= this->out_.capacity())
  {
  }

  if (_bytes.size() > this->out_.capacity())
    ;
  std::copy(_bytes.begin(), _bytes.end(), this->out_.write_buffer().begin());
  ssize_t bytes = ::send(this->socket_, _bytes.data(), _bytes.size(), 0);
  if (bytes >= 0)
  {
    // try send rem-
    if (bytes < _bytes.size()) this->send(_bytes.subspan(bytes));
  }
  else
  {
    switch (errno)
    {
#if EAGAIN != EWOULDBLOCK
    case EWOULDBLOCK:
#endif
    case EAGAIN:

    case ECONNRESET:

    case EACCES:
    case EALREADY:
    case EDESTADDRREQ:
    case EFAULT:
    case EBADF:
    case EINTR:
    case EINVAL:
    case EISCONN:
    case EMSGSIZE:
    case ENOTSOCK:
    case ENOBUFS:
    case ENOMEM:
    case ENOTCONN:
    case EOPNOTSUPP:
    case EPIPE:
    }
  }
};

using ParserResult = protocol::ParserResult;
using Request = protocol::forwarding::Request;

constexpr auto not_found = as_bytes<115>("HTTP/1.1 404 Not Found\r\n"
                                         "Content-Type: text/plain; charset=utf-8\r\n"
                                         "Content-Length: 32\r\n"
                                         "\r\n"
                                         "Reverse proxy\npage not found");

constexpr auto invalid_headers = as_bytes<125>("HTTP/1.1 400 Bad Request\r\n"
                                               "Content-Type: text/plain; charset=utf-8\r\n"
                                               "Content-Length: 36\r\n"
                                               "\r\n"
                                               "Reverse proxy\ninvalid request header");

bool TcpServer::Client::get_host(std::span<const char> bytes)
{
  if (this->host_ != nullptr) return true;

  size_t header_size = Request::parse(bytes, this->ctx_);
  switch (this->ctx_.result)
  {
  case ParserResult::NeedMoreData:
    if (bytes.size() < this->max_header_size) return false; // do nothing go back to listening
    else [[fallthrough]];                                   // header bigger than max header size;

  case ParserResult::Invalid:
    this->send(invalid_headers);
    this->parent.drop_child(this->uuid());
    return false;

  case ParserResult::Complete:
    auto req = this->ctx_.construct();
    auto it = this->parent.forwarding_.find(req.host);
    if (it == this->parent.forwarding_.cend())
    { // no host
      this->send(not_found);
      this->parent.drop_child(this->uuid());
      return false;
    }
    else if ((this->host_ = it->second) == nullptr)
    {
      Logger::crit("Assertion failed!\nForwarding table should't contain a nullptr as a host");
      throw std::runtime_error("nullptr host in forwarding table");
    }

    this->remaining = header_size + req.content_length;
    return true;
  }

  std::unreachable();
}

void TcpServer::Client::notify_read()
{
  auto log = Logger::get(std::format("TCP Client IP={}", this->ip_));
  log.debug("notify read");

  while (this->recv())
  {
    auto bytes = this->in_.read_buffer();

    if (!this->get_host(char_cast(bytes))) return;

    // if a host has been find at least `header_size` should remain
    assert(this->remaining != 0);

    if (bytes.size() < this->remaining)
    { // not all request has been sent
      this->host_->send(bytes);
      this->in_.discard(bytes.size());
      this->remaining -= bytes.size();
    }
    else
    { // end of request
      this->host_->send(bytes.subspan(0, this->remaining));
      this->in_.discard(this->remaining);

      this->ctx_.reset();
      this->host_ = nullptr;
      this->remaining = 0;
    }
  }
}

void TcpServer::Client::notify_write() {}

void TcpServer::Client::notify_half_close()
{
  auto log = Logger::get(std::format("TCP Client IP={}", this->ip_), [] { return strerror(errno); });
  log.warn("half closed...");
  this->parent.drop_child(this->uuid_);
}

void TcpServer::Client::notify_close()
{
  auto log = Logger::get(std::format("TCP Client IP={}", this->ip_));
  log.log("closed...");
  this->parent.drop_child(this->uuid_);
}

void TcpServer::Client::notify_error()
{
  auto log = Logger::get(std::format("TCP Client IP={}", this->ip_), [] { return strerror(errno); });
  log.error("error");
  this->parent.drop_child(this->uuid_);
}
