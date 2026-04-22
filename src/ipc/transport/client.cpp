#include "../../logger/logger.hpp"
#include "transport.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>

namespace ipc
{

template <protocol::Protocol P>
TransportClient<P>::TransportClient(const std::string _ip, const int _port) : ip_(_ip), port_(_port)
{
  auto log = Logger::New();
  int code;

  this->addr_.sin_family = AF_INET;
  this->addr_.sin_port = htons(_port);

  code = inet_pton(AF_INET, _ip.c_str(), &this->addr_.sin_addr);
  if (code == 0)
  {
    log.crit("invalid ip (IP={}): {}", _ip, strerror(errno));
    throw "InvalidIpFormat";
  }
  if (code < 0)
  {
    log.crit("resolveing IP failed (IP={}): {}", _ip, strerror(errno));
    throw "FailedToResolveIp";
  }

  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ < 0)
  {
    log.crit("socket creation failed: {}", strerror(errno));
    throw "SocketCreationException";
  }

  log.info("socket created (fd={})", this->socket_);
  return;
}

template <protocol::Protocol P> TransportClient<P>::~TransportClient()
{
  if (this->socket_ > 0) close(this->socket_);
}

template <protocol::Protocol P> void TransportClient<P>::connect()
{
  auto log = Logger::New();

  if (::connect(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof this->addr_))
  {
    log.crit("connection failed (server={}:{}): {}", this->ip_, this->port_, strerror(errno));
    throw "ConnectionException";
  }

  log.info("connected (server={}:{})", this->ip_, this->port_);
  return;
}

template <protocol::Protocol P> P::Response TransportClient<P>::transmit(const P::Request& _request)
{
  auto log = Logger::New();
  const std::string str(_request);

  typename P::Response response;
  ssize_t bytes;

  bytes = send(this->socket_, str.c_str(), str.size(), 0);
  if (bytes < 0)
  {
    log.crit("send failed: {}", strerror(errno));
    throw "SendingException";
  }

  bytes = recv(this->socket_, this->buffer_.write_begin(), this->buffer_.write_length(), 0);
  if (bytes < 0)
  {
    log.crit("receive failed: {}", strerror(errno));
    throw "ReceivingException";
  }

  return response;
}

} // namespace ipc
