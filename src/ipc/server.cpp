#include "../logger/logger.hpp"
#include "transport.hpp"
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CLIENTS 1
#define BUFF_SIZE   4096

namespace ipc
{

template <Protocol P>
TransportServer<P>::TransportServer(const int _port, const protocol::Handler<P>& _handler) : handler_(_handler)
{
  const auto log = Logger::New();

  log.info("Creating socket for transport server.");
  this->socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (this->socket_ < 0)
  {
    log.error("Failed to create new socket.");
    log.error(std::strerror(errno));
    throw "SocketCreationException";
  }

  log.info("Setting transport server's socket options.");
  int opts = 1;
  if (setsockopt(this->socket_, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opts, sizeof opts))
  {
    log.error("Failed to set socket options.");
    log.error(std::strerror(errno));
    throw "SocketOptionsException";
  }

  this->addr_.sin_family = AF_INET;
  this->addr_.sin_port = htons(_port);
  this->addr_.sin_addr.s_addr = INADDR_ANY;

  log.info("Binding transport server to port {}.", _port);
  int res = bind(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof this->addr_);
  if (res)
  {
    log.error("Failed to bind transport server on port {}.", _port);
    log.error(std::strerror(errno));
    throw "BindingException";
  }
}

template <Protocol P> TransportServer<P>::~TransportServer()
{
  if (this->socket_ > 0) ::close(this->socket_);
};

template <Protocol P> void TransportServer<P>::listen()
{
  const auto log = Logger::New();

  log.info("Transport server listening on port {}.", ntohs(this->addr_.sin_port));
  int res = ::listen(this->socket_, MAX_CLIENTS);
  if (res < 0)
  {
    log.error("Transport server failed to listen on port {}.", ntohs(this->addr_.sin_port));
    log.error(std::strerror(errno));
    throw "ListeningException";
  }

  this->running_ = true;
  while (this->running_)
  {
    socklen_t len = sizeof this->addr_;
    int client_socket = accept(this->socket_, reinterpret_cast<sockaddr*>(&this->addr_), &len);

    log.info("New client connetion.");
    if (client_socket < 0)
    {
      log.warn("Client connetion failed.");
      continue;
    }

    while (this->running_)
    {
      typename P::Request request;
      typename P::Response response;
      ssize_t bt;

      bt = P::recv(client_socket, request);
      if (bt == 0) // client disconnected
      {
        log.info("Client disconnected.");
        break;
      }

      if (bt < 0)
      {
        log.warn("Transport server failed to receive client message.");
        log.warn(strerror(errno));
        continue;
      }

      response = this->handler_(request);
      bt = P::send(client_socket, response);
      if (bt < 0)
      {
        log.error("Transport server failed to send response to client.");
        log.error(strerror(errno));
        throw "SendingException";
      }
    }

    log.info("Closing client socket.");
    close(client_socket);
  }
}

} // namespace ipc
