#include "../../logger/logger.hpp"
#include "transport.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace ipc
{

template <protocol::Protocol P>
TransportServer<P>::TransportServer(const std::string& _ip, const int _port, const protocol::Handler<P> _handler,
                                    const std::string _bad_request)
    : ip_(_ip), port_(_port), handler_(_handler), bad_request_(_bad_request)
{
  auto log = Logger::get("IPC Server", [] { return strerror(errno); });

  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ <= 0)
  {
    log.crit("socket creation failed");
    throw std::runtime_error("SocketCreationException");
  }

  log.debug("socket created (fd={})", this->socket_);

  int opts = 1;
  if (setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opts, sizeof opts))
  {
    log.crit("socket option configuration failed");
    throw std::runtime_error("socket option configuration failed");
  }

  this->addr_.sin_family = AF_INET;
  this->addr_.sin_port = htons(_port);
  this->addr_.sin_addr.s_addr = INADDR_ANY;

  if (inet_pton(AF_INET, _ip.c_str(), &this->addr_.sin_addr) != 1)
  {
    log.crit("IP {} resolution failed", this->ip_);
    throw std::runtime_error("ip resolution failed");
  }

  if (bind(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof this->addr_))
  {
    log.crit("socket bind failed");
    throw std::runtime_error("socket binding failed");
  }

  log.debug("socket bind ({}:{})", _ip, _port);
  log.info("successfully created ready to listen");
  return;
}

template <protocol::Protocol P> TransportServer<P>::~TransportServer()
{
  if (this->socket_ > 0) ::close(this->socket_);
};

template <protocol::Protocol P> void TransportServer<P>::listen()
{
  auto log = Logger::get(std::format("IPC Server ip={}:{}", this->ip_, this->port_));

  int code = ::listen(this->socket_, SOMAXCONN);
  if (code < 0)
  {
    log.crit("socket listen failed");
    throw std::runtime_error("socket listen failed");
  }

  log.info("socket listen (addr={}:{})", this->ip_, this->port_);

  this->listening_ = true;
  socklen_t len = sizeof this->addr_;

  while (this->listening_)
  {
    TransportServer<P>::Client client = {
        .socket = accept(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), &len),
        .port = ntohs(this->addr_.sin_port)};

    inet_ntop(AF_INET, &this->addr_.sin_addr, client.ip, sizeof this->addr_.sin_addr);

    if (client.socket < 0)
    {
      log.error("connection accept failed");
      continue;
    }
    log.info("connection accepted (client={}:{})", client.ip, client.port);

    while (this->listening_)
    {
      ssize_t bytes;
      typename P::Request request;
      typename P::Response response;

      // might want to grow buf
      bytes = recv(client.socket, client.connection_buffer.write(), client.connection_buffer.capacity(), 0);
      if (bytes == 0)
      {
        log.info("disconnection (client={}:{})", client.ip, client.port);
        goto disconnect_client; // more explicit than break
      }
      if (bytes < 0)
      {
        log.error("recv failed (fd={}): {}", client.socket, strerror(errno));
        goto disconnect_client;
      }

      log.info("request received (fd={}, size={})", client.socket, bytes);
      client.connection_buffer.acknowledge(bytes);

      bytes = P::Request::parse(client.connection_buffer.read(), client.parsing_ctx);
      switch (client.parsing_ctx.result)
      {
      case protocol::ParserResult::Invalid: {
        log.warn("request invalid: {}\n{}", client.parsing_ctx.error_msg,
                 std::string(client.connection_buffer.read().begin(), client.connection_buffer.read().end()));
        if (send(client.socket, this->bad_request_.c_str(), this->bad_request_.size(), 0) <= 0)
          log.error("send failed: {}", strerror(errno));

        goto disconnect_client;
      }

      case protocol::ParserResult::NeedMoreData: {
        client.connection_buffer.discard(bytes);
        continue;
      }

      case protocol::ParserResult::Complete: {
        log.info("request parsed successfully", client.socket);
        client.connection_buffer.clear(); // not sure if this is right
        break;
      }
      }

      request = client.parsing_ctx.construct();
      response = this->handler_(request);

      std::string str(response);
      bytes = send(client.socket, str.c_str(), str.size(), 0);
      if (bytes <= 0)
      {
        log.error("send failed: {}", client.socket, strerror(errno));
        break;
      }

      log.info("sended (bytes={}) to (client={}:{}) on (socket={})", bytes, client.ip, client.port, client.socket);
    }

  disconnect_client:
    log.info("disconnection (fd={})", client.socket);
    close(client.socket);
  }

  return;
}

} // namespace ipc
