#include "tls.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

using namespace asio;
using namespace tls;

Socket::Socket(const tcp::Socket&& socket, SSL_CTX* const _ctx)
    : SocketBase{static_cast<const SocketBase&&>(socket)}
{
  this->ctx_.state = Context::handshake;
  this->ctx_.ssl = SSL_new(_ctx);

  if (this->ctx_.ssl == nullptr)
  {
  }

  if (SSL_set_fd(this->ctx_.ssl, this->fd_) == 0)
  {
  }

  return;
}

Executor::Operation Socket::read(std::span<std::byte> buffer, std::pair<std::size_t, Error>& value)
{
  Reactor::Events events = (*this->reactor_)[this->fd_];

  // conditions
  if (events.error)
  {
    value.second = Error::system_error;
    return Executor::Operation::end;
  }
  else if (events.socket)
  {
    value.second = Error::socket_event;
    return Executor::Operation::end;
  }
  else if (!events.read)
  {
    value.second = Error::none;
    return Executor::Operation::pri_read;
  }

  // action
  if (this->ctx_.state == Context::handshake)
  {
    int ret_code = SSL_accept(this->ctx_.ssl);
    if (ret_code == 0)
    {
      value.first = 0;
      value.second = Error::eof; // might not be the best err
      return Executor::Operation::end;
    }
    else if (ret_code <= 0)
    {
      switch (SSL_get_error(this->ctx_.ssl, ret_code))
      { // TODO later
      case SSL_ERROR_NONE:
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
      default:
      }
    }

    this->ctx_.state = Context::established;
  }

  int ret_code = SSL_read(this->ctx_.ssl, buffer.data(), buffer.size());

  // results
  if (ret_code > 0)
  {
    value.first = ret_code;
    value.second = Error::none;
    return Executor::Operation::end;
  }
  else if (ret_code == 0)
  {
    // TODO
  }
  else switch (SSL_get_error(this->ctx_.ssl, ret_code))
    {
    }

  return Executor::Operation::end;
}
