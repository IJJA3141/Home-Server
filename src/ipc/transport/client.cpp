#include "../../logger/logger.hpp"
#include "transport.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace ipc
{

template <protocol::Protocol P>
TransportClient<P>::TransportClient(const std::string _ip, const int _port) : ip_(_ip), port_(_port)
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
    throw std::runtime_error("socket creation failed");
  }

  log.info("creation successful ready to connect");
  return;
}

template <protocol::Protocol P> TransportClient<P>::~TransportClient()
{
  if (this->socket_ > 0) close(this->socket_);
}

template <protocol::Protocol P> void TransportClient<P>::connect()
{
  auto log = Logger::get("IPC Client", [] { return strerror(errno); });

  if (::connect(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof this->addr_))
  {
    // ECONNREFUSED might use for reconnection
    log.crit("connection (server={}:{}) failed", this->ip_, this->port_, strerror(errno));
    throw std::runtime_error("connection error");
  }

  log.info("connected (server={}:{})", this->ip_, this->port_);
  return;
}

template <protocol::Protocol P> P::Response TransportClient<P>::transmit(const P::Request& _request)
{
  auto log = Logger::get("IPC Client", [] { return strerror(errno); });
  const std::string str(_request);

  typename P::Response response;
  ssize_t bytes;

  bytes = send(this->socket_, str.c_str(), str.size(), 0);
  if (bytes < 0)
  {
    log.crit("send failed: {}", strerror(errno));
    throw std::runtime_error("send failed");
  }

  typename P::Response::ParserContext ctx;

  while (true)
  {
    bytes = recv(this->socket_, this->buffer_.write(), this->buffer_.capacity(), 0);
    if (bytes < 0)
    {
      log.crit("receive failed");
      throw std::runtime_error("receive failed");
    }
    else if (bytes == 0) log.warn("received 0 bytes");
    else this->buffer_.acknowledge(bytes);

    log.debug("received {} bytes", bytes);
    log.debug("buffer={}", std::string(this->buffer_.read().begin().base()));

    // bytes = P::Response::parse(this->buffer_.read(), this->parser_ctx_);
    bytes = P::Response::parse(this->buffer_.read(), ctx);
    this->buffer_.discard(bytes);
    log.debug("discarded {} bytes", bytes);
    log.debug("buffer={}", std::string(this->buffer_.read().begin().base()));

    // switch (this->parser_ctx_.result)
    switch (ctx.result)
    {
    case protocol::ParserResult::Invalid:
      log.crit("received an invalid response {}", this->buffer_.read());
      throw std::runtime_error("parsing failed");

    case protocol::ParserResult::NeedMoreData:
      log.debug("need more data");
      continue;

    case protocol::ParserResult::Complete:
      log.debug("successfully parsed ipc response");
      // return this->parser_ctx_.construct();
      return ctx.construct();
    }
  }
}

} // namespace ipc
