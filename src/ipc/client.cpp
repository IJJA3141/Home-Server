#include "../logger/logger.hpp"
#include "transport.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace ipc
{

template <Protocol P> TransportClient<P>::TransportClient(const int _port, const char* _ip)
{
  const auto log = Logger::New();

  this->addr_.sin_family = AF_INET;
  this->addr_.sin_port = htons(_port);

  log.info("resolving ip {}", _ip);
  int res = inet_pton(AF_INET, _ip, &this->addr_.sin_addr);
  if (res == 0)
  {
    log.error("invalid ip format.");
    throw "InvalidIpFormat";
  }
  else if (res < 0)
  {
    log.error("Transport client failed to resolve ip.");
    log.error(std::strerror(errno));
    throw;
  }

  log.info("Creating socket for transport client.");
  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ < 0)
  {
    log.error("Failed to create new socket.");
    log.error(std::strerror(errno));
    throw "SocketCreationException";
  }
}

template <Protocol P> TransportClient<P>::~TransportClient()
{
  if (this->socket_ > 0) close(this->socket_);
};

template <Protocol P> void TransportClient<P>::connect()
{
  const auto log = Logger::New();

  char buf[16];
  inet_ntop(AF_INET, &this->addr_.sin_addr, buf, sizeof buf);
  log.info("Transport client connecting to {}:{}.", buf, ntohs(this->addr_.sin_port));
  if (::connect(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof this->addr_))
  {
    log.error("Transport client failed to connect to {}:{}.", buf, ntohs(this->addr_.sin_port));
    log.error(std::strerror(errno));
    throw "ConnectionException";
  }
};

template <Protocol P> P::Response TransportClient<P>::transmit(P::Request _request)
{
  const auto log = Logger::New();

  typename P::Response response;
  ssize_t bt;

  log.info("Transport client sending request.");
  bt = P::send(this->socket_, _request);
  if (bt < 0)
  {
    log.error("Transport client failed to send request.");
    log.error(std::strerror(errno));
    throw "SendingException";
  }

  log.info("Transport client receiving response.");
  bt = P::recv(this->socket_, response);
  if (bt < 0)
  {
    log.error("Transport client failed to receive response.");
    log.error(std::strerror(errno));
    throw "SendingException";
  }

  return response;
};

} // namespace ipc
