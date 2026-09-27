#include "../common/utils.hpp"
#include "server.hpp"
#include <contracts>

#include <arpa/inet.h>
#include <cassert>
#include <cerrno>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <system_error>

using add = protocol::ipcp::Request::add;
using snd = protocol::ipcp::Request::snd;
using del = protocol::ipcp::Request::del;

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

  for (ipc::IClient* host : this->opened_hosts_)
    host->transmit({del(this->uuid_)});

  log.info("disconnected...");
}

bool TcpServer::Client::recv()
{
  auto in = this->in_.write_buffer();
  ssize_t bytes;
  while (this->in_.capacity() > 0 && (bytes = ::recv(this->socket_, in.data(), in.size(), 0) >= 0))
  {
    this->in_.acknowledge(bytes);
  }

  if (bytes < 0)
  {
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
  }

  throw std::runtime_error("in buffer was full");
}

// TODO
void TcpServer::Client::send(std::span<const std::byte> _bytes) {
  // #if EAGAIN != EWOULDBLOCK
  //     case EWOULDBLOCK:
  // #endif
  //     case EAGAIN:
  //
  //     case ECONNRESET:
  //
  //     case EACCES:
  //     case EALREADY:
  //     case EDESTADDRREQ:
  //     case EFAULT:
  //     case EBADF:
  //     case EINTR:
  //     case EINVAL:
  //     case EISCONN:
  //     case EMSGSIZE:
  //     case ENOTSOCK:
  //     case ENOBUFS:
  //     case ENOMEM:
  //     case ENOTCONN:
  //     case EOPNOTSUPP:
  //     case EPIPE:
};

void TcpServer::Client::send_error()
{
  switch (this->ctx_.result)
  {
  case protocol::ParserResult::NeedMoreData:
  case protocol::ParserResult::Invalid:
    this->send(as_bytes<125>("HTTP/1.1 400 Bad Request\r\n"
                             "Content-Type: text/plain; charset=utf-8\r\n"
                             "Content-Length: 36\r\n"
                             "\r\n"
                             "Reverse proxy\ninvalid request header"));
    break;

  case protocol::ParserResult::Complete:
    this->send(as_bytes<115>("HTTP/1.1 404 Not Found\r\n"
                             "Content-Type: text/plain; charset=utf-8\r\n"
                             "Content-Length: 32\r\n"
                             "\r\n"
                             "Reverse proxy\npage not found"));
    break;
  }

  this->parent.drop_child(this->uuid_);
}

TcpServer::Client::State TcpServer::Client::find_host()
{
  using Request = protocol::forwarding::Request;

  if (this->host_ != nullptr) return FORWARIND;

  auto chars = char_cast(this->in_.read_buffer());
  size_t header_size = Request::parse(chars, this->ctx_);

  switch (this->ctx_.result)
  {
    using enum protocol::ParserResult;
  case NeedMoreData:
    if (chars.size() < this->max_header_size_) return PARSING;
    else [[fallthrough]];

  case Invalid:
    return ERROR;

  case Complete:
    auto req = this->ctx_.construct();
    auto it = this->parent.forwarding_.find(req.host);
    if (it == this->parent.forwarding_.cend()) return ERROR;
    else if (it->second == nullptr)
    {
      Logger::crit("Assertion failed!\nForwarding table should't contain a nullptr as a host");
      throw std::runtime_error("nullptr host in forwarding table");
    }

    this->host_ = it->second;
    if (!this->opened_hosts_.contains(this->host_))
    {
      this->host_->transmit(
          {add{this->uuid_, this->addr_.sin_port, this->addr_.sin_addr.s_addr, this->connection_type}});
      this->opened_hosts_.insert(this->host_);
    }
    this->remaining_ = header_size + req.content_length;
  }

  return FORWARIND;
}

void TcpServer::Client::notify_read()
{
  auto log = Logger::get(std::format("TCP Client IP={}", this->ip_));
  log.debug("notify read");

  // can read one more time
  while (this->recv())
  {
  skip_recv:
    switch (this->find_host())
    {
    case PARSING:
      continue;

    case FORWARIND:
      break;

    case ERROR:
      this->send_error();
      return;
    }

    auto bytes = this->in_.read_buffer();
    if (this->remaining_ > bytes.size())
    { // more incomming
      this->host_->transmit({snd(this->uuid_, bytes)});
      this->remaining_ -= bytes.size();
      this->in_.discard(bytes.size());
      continue; // need recv
    }
    else
    { // final segment
      this->host_->transmit({snd(this->uuid_, bytes.subspan(0, this->remaining_))});
      this->in_.discard(this->remaining_);

      // reset
      this->ctx_.reset();
      this->remaining_ = 0;
      this->host_ = nullptr;
      goto skip_recv; // restart with current buffer
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
