#include "../../common/logger.hpp"
#include "../../common/utils.hpp"
#include "../../protocol/ipcp/ipcp.hpp"
#include "tcp.hpp"
#include <arpa/inet.h>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

using namespace ipc::tcp;
Client::Client(const std::string& _ip, const int _port) : ip_(_ip), port_(_port), socket_(-1)
{
  auto log = Logger::get("IPC Client", [] { return strerror(errno); });
  int code;

  this->addr_.sin_family = AF_INET;
  this->addr_.sin_port = htons(_port);

  code = inet_pton(AF_INET, _ip.c_str(), &this->addr_.sin_addr);
  if (code == 0)
  {
    log.crit("invalid ip addr (IP={})", _ip);
    throw std::runtime_error("invalid ip addr");
  }

  if (code < 0)
  {
    log.crit("resolving IP failed (IP={})", _ip);
    throw std::runtime_error("resolving ip failed");
  }

  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ < 0)
  {
    log.crit("socket creation failed: {}", strerror(errno));
    throw std::system_error(errno, std::system_category(), "socket creation failed");
  }

  log.info("creation successful ready to connect");
  return;
}

Client::~Client()
{
  if (this->socket_ >= 0) close(this->socket_);
}

void Client::connect()
{
  auto log =
      Logger::get(std::format("IPC Client (ip={}:{})", this->ip_, this->port_), [] { return strerror(errno); });

  if (::connect(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof this->addr_))
  {
    // ECONNREFUSED might use for reconnection
    log.crit("connection failed");
    throw std::system_error(errno, std::system_category(), "connection error");
  }

  log.info("connected");
  return;
}

void Client::transmit(protocol::ipcp::Request _req)
{
  auto bytes = this->out_.write_buffer();
  auto sent = _req.insert(bytes);
  this->out_.acknowledge(sent);
  this->notify_write();
}

void Client::notify_write()
{
  auto bytes = this->out_.write_buffer();
  ssize_t sent;
  while (this->out_.size() > 0 && (sent = ::send(this->socket_, bytes.data(), bytes.size(), 0) > 0))
  {
    this->out_.discard(sent);
  }

  if (sent < 0)
  {
    switch (errno)
    {
#if EAGAIN != EWOULDBLOCK
    case EWOULDBLOCK:
#endif
    case EAGAIN:
      break;

    case ECONNRESET:
    case EINTR:
    case ENOTCONN:
      this->notify_half_close(); // try to reconnect
      this->notify_write();
      break;

    case ENOBUFS:
    case EACCES:
    case EALREADY:
    case EBADF:
    case EDESTADDRREQ:
    case EFAULT:
    case EINVAL:
    case EISCONN:
    case EMSGSIZE:
    case ENOMEM:
    case ENOTSOCK:
    case EOPNOTSUPP:
    case EPIPE:
      throw std::system_error(errno, std::system_category(), "send failed");
    }
  }
}

void Client::notify_half_close()
{
  this->connect();
  this->notify_close();
}

void Client::notify_close()
{
  this->connect();
  // might add move
}

void Client::notify_error()
{
  this->connect();
  this->notify_close();
}

bool Client::recv()
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

  return true;
}

void Client::notify_read()
{
  using Response = protocol::ipcp::Response;

  while (this->recv())
  {
    auto chars = char_cast(this->in_.read_buffer());
    auto bytes = Response::parse(chars, this->ctx_);
    switch (this->ctx_.result)
    {
    case protocol::ParserResult::NeedMoreData:
      continue;

    case protocol::ParserResult::Invalid:
      throw std::runtime_error("invalid request");

    case protocol::ParserResult::Complete:
      auto req = this->ctx_.construct();
      std::visit(overloads{
                     [](Response::snd req) {
req.client_id;
          },
                     [](Response::del req) {},
                 },
                 req.request);
      break;
    }
  }
}
