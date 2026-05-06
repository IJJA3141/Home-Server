#include "../logger/logger.hpp"
#include "reverse_proxy.hpp"
#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/types.h>

std::atomic<size_t> TlsServer::SSL_LIB_HANDLE = 0;
std::atomic<bool> TlsServer::SSL_LIB_INIT = false;

void load_lib_ssl()
{
  SSL_library_init();
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  TlsServer::SSL_LIB_INIT = true;
}

TlsServer::TlsServer(const std::string& ip, uint16_t port, const std::filesystem::path& _cert,
                     const std::filesystem::path& _key, const Handler request_handler)
    : TcpServer(ip, port, request_handler)
{
  auto log = Logger::New();

  if (!TlsServer::SSL_LIB_INIT) load_lib_ssl();

  this->ctx_ = SSL_CTX_new(TLS_method());
  if (!this->ctx_)
  {
    log.crit("");
    throw "";
  }

  int c = SSL_CTX_use_certificate_file(this->ctx_, _cert.c_str(), SSL_FILETYPE_PEM);
  if (c != 1)
  {
    log.crit("");
    throw "";
  }

  c = SSL_CTX_use_PrivateKey_file(this->ctx_, _key.c_str(), SSL_FILETYPE_PEM);
  if (c != 1)
  {
    log.crit("");
    throw "";
  }

  c = SSL_CTX_check_private_key(this->ctx_);
  if (c != 1)
  {
    log.crit("");
    throw "";
  }

  TlsServer::SSL_LIB_HANDLE++;
}

TlsServer::~TlsServer()
{
  if (--TlsServer::SSL_LIB_HANDLE) return;

  OPENSSL_cleanup();
  TlsServer::SSL_LIB_INIT = false;
}

void TlsServer::accept()
{
  try
  {
    new Client(this->listening_socket_, this->epoll_fd_, this->ctx_, this->request_handler_);
  }
  catch (const std::exception& e)
  {
  }
}

TlsServer::Client::Client(const int listening_socket, const int epoll, SSL_CTX* const _ctx,
                          const Handler request_handler)
    : TcpServer::Client(listening_socket, epoll, request_handler)
{
  auto log = Logger::New();

  this->ssl_ = SSL_new(_ctx);
  if (!this->ssl_)
  {
    log.error("a");
    throw "";
  }

  if (!SSL_set_fd(this->ssl_, this->socket))
  {
    log.error("n");
    throw "";
  }

  int flag = fcntl(this->socket, F_GETFL);
  if (flag == -1)
  {
    log.error("c");
    throw "";
  }

  // set to blocking for ssl handshake
  if (fcntl(this->socket, F_SETFL, flag & ~O_NONBLOCK) == -1)
  {
    log.error("d");
    throw "";
  }

  int err = SSL_accept(this->ssl_);
  if (err <= 0)
  {
    log.error("ssl accept failed");
    ERR_print_errors_fp(stdout);
    throw "";
  }

  if (fcntl(this->socket, F_SETFL, flag))
  {
    log.error("d");
    throw "";
  }
}

TlsServer::Client::~Client()
{
  SSL_shutdown(this->ssl_);
  SSL_free(this->ssl_);
}

ssize_t TlsServer::Client::recv(int __fd, void* __buf, size_t __n, int __flags)
{
  auto log = Logger::New();

  ssize_t bytes = SSL_read(this->ssl_, __buf, __n);
  if (bytes <= 0)
  {
    log.error("ssl recv failed");
    ERR_print_errors_fp(stdout);
  }

  return bytes;
}

ssize_t TlsServer::Client::send(int __fd, const void* __buf, size_t __n, int __flags)
{
  auto log = Logger::New();

  ssize_t bytes = SSL_write(this->ssl_, __buf, __n);
  if (bytes <= 0)
  {
    log.error("ssl send failed");
    ERR_print_errors_fp(stdout);
  }

  return bytes;
}
