#include "../common/logger.hpp"
#include "reverse_proxy.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdexcept>

TlsServer::Client::Client(int listening_socket, int epoll, SSL_CTX* ctx,
                          const protocol::Handler<ForwardPolicy>& handler)
    : TcpServer::Client{listening_socket, epoll, handler}, handshake_established{false}
{
  auto log = Logger::get(std::format("TLS Client {}", this->ip),
                         [] { return ERR_error_string(ERR_get_error(), nullptr); });

  if ((this->ssl_ = SSL_new(ctx)) == nullptr)
  {
    log.error("SSL session creation failed");
    throw std::runtime_error("SSL session creation failed");
  }

  if (!SSL_set_fd(this->ssl_, this->socket))
  {
    SSL_free(this->ssl_);
    log.error("SSL session association with socket (fd={}) failed", this->socket);
    throw std::runtime_error("SSL session association with socket failed");
  }

  this->handshake();
}

TlsServer::Client::~Client()
{
  SSL_shutdown(this->ssl_);
  SSL_free(this->ssl_);
  Logger::info("ssl shutdown");
}

void TlsServer::Client::handshake()
{
  int err = SSL_accept(this->ssl_);
  if (err != 1) this->handle_error(err);
  else
  {
    auto log = Logger::get(std::format("TLS Client {}", this->ip));
    this->handshake_established = true;
    log.info("ssl handshake succeded");
  }
}

void TlsServer::Client::handle_error(int err)
{
  auto log = Logger::get(std::format("TLS Client {}", this->ip),
                         [] { return ERR_error_string(ERR_get_error(), nullptr); });

  switch (SSL_get_error(this->ssl_, err))
  {
  // SSL_ERROR_NONE The TLS/SSL I/O operation completed.  This result code is
  // returned if and only if ret > 0.
  case SSL_ERROR_NONE:
    break;

  // SSL_ERROR_ZERO_RETURN The TLS/SSL peer has closed the connection for
  // writing by sending the close_notify alert.  No more data can be read.
  // Note that SSL_ERROR_ZERO_RETURN does not necessarily indicate that the
  // underlying transport has been closed.
  case SSL_ERROR_ZERO_RETURN:
    this->connection_failed = true; // ?
    break;

  // SSL_ERROR_WANT_READ, SSL_ERROR_WANT_WRITE The operation did not complete
  // and can be retried later.
  case SSL_ERROR_WANT_READ:
  case SSL_ERROR_WANT_WRITE:
    break;

  // SSL_ERROR_WANT_CONNECT, SSL_ERROR_WANT_ACCEPT The operation did not
  // complete; the same TLS/SSL I/O function should be called again later.
  // The underlying BIO was not con‐ nected yet to the peer and the call
  // would block in connect()/accept(). The SSL function should be called
  // again when the con‐ nection is established. These messages can only
  // appear with a BIO_s_connect() or BIO_s_accept() BIO, respectively.  In
  // order to find out, when the connection has been successfully
  // established, on many platforms select() or poll() for writing on the
  // socket file descriptor can be used.
  case SSL_ERROR_WANT_ACCEPT:
    this->handshake_established = false;
    break;

  default:
    log.crit("unrecoverable TLS/SSL error");
    throw std::runtime_error("unrecoverable TLS/SSL error");
  }
}

ssize_t TlsServer::Client::recv(int __fd, void* __buf, size_t __n)
{
  if (!this->handshake_established) [[unlikely]]
  {
    this->handshake();
    return 0;
  }

  ssize_t bytes = SSL_read(this->ssl_, __buf, __n);
  if (bytes <= 0) this->handle_error(bytes);
  return bytes;
}

ssize_t TlsServer::Client::send(int __fd, const void* __buf, size_t __n, int __flags)
{
  if (!this->handshake_established) [[unlikely]]
  {
    handshake();
    return 0;
  }

  ssize_t bytes = SSL_write(this->ssl_, __buf, __n);
  if (bytes <= 0) this->handle_error(bytes);
  return bytes;
}
