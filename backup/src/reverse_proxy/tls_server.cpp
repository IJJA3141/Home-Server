#include "../logger/logger.hpp"
#include "reverse_proxy.hpp"
#include <fcntl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/types.h>

std::atomic<int> TlsServer::SSL_LIB_INIT = 0;

void load_lib_ssl()
{
  if (TlsServer::SSL_LIB_INIT++ == 0)
  {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    // SSL_load_error_strings();
  }
}

void unload_lib_ssl()
{
  if (--TlsServer::SSL_LIB_INIT == 0) OPENSSL_cleanup();
}

TlsServer::TlsServer(const std::string& ip, uint16_t port, const std::filesystem::path& _cert,
                     const std::filesystem::path& _key, const Handler request_handler)
    : TcpServer(ip, port, request_handler)
{
  load_lib_ssl();

  // auto log = Logger::Instance([&](std::string args) -> std::string {
  //   unsigned long err = ERR_get_error();
  //   if (err) return std::format("(TLS Server)\t{}\t{}", args, ERR_error_string(err, nullptr));
  //   else return "(TLS Server)\t" + args;
  // });

  auto log = Logger::get("TSL Server", [] { return ERR_error_string(ERR_get_error(), nullptr); });

  this->ctx_ = SSL_CTX_new(TLS_method());
  if (!this->ctx_)
  {
    unload_lib_ssl();
    log.crit("SSL context creation failed");
    throw std::runtime_error("SSL context creation failed");
  }

  int c = SSL_CTX_use_certificate_file(this->ctx_, _cert.c_str(), SSL_FILETYPE_PEM);
  if (c != 1)
  {
    unload_lib_ssl();
    SSL_CTX_free(this->ctx_);
    log.crit("ssl certificate loading {} failed", _cert);
    throw std::runtime_error("ssl certificate loading failed");
  }

  c = SSL_CTX_use_PrivateKey_file(this->ctx_, _key.c_str(), SSL_FILETYPE_PEM);
  if (c != 1)
  {
    unload_lib_ssl();
    SSL_CTX_free(this->ctx_);
    log.crit("ssl key {} loading failed", _key);
    throw std::runtime_error("ssl key loading failed");
  }

  c = SSL_CTX_check_private_key(this->ctx_);
  if (c != 1)
  {
    unload_lib_ssl();
    SSL_CTX_free(this->ctx_);
    log.crit("certificate {} doesn't match key {}", _cert, _key);
    throw std::runtime_error("certificate and key don't match");
  }

  log.info("creation successful, ready to listen");
}

TlsServer::~TlsServer()
{
  SSL_CTX_free(this->ctx_);
  unload_lib_ssl();
  Logger::debug("(TLS Server) closed...");
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
  // auto log = Logger::New();
  // auto log = Logger::get(std::format("TLS Client IP={}", this->ip),
  //                        [] { return ERR_error_string(ERR_get_error(), nullptr); });
  //
  // log.debug("creating ssl context...");
  // this->ssl_ = SSL_new(_ctx);
  // if (!this->ssl_)
  // {
  //   log.error("SSL session creation failed");
  //   throw std::runtime_error("SSL session creation failed");
  // }
  //
  // log.debug("assigning socket to ssl context...");
  // if (!SSL_set_fd(this->ssl_, this->socket))
  // {
  //   SSL_free(this->ssl_);
  //   log.error("SSL session association with socket (fd={}) failed", this->socket);
  //   throw std::runtime_error("SSL session association with socket failed");
  // }
  //
  // int flag = fcntl(this->socket, F_GETFL);
  // if (flag == -1)
  // {
  //   SSL_free(this->ssl_);
  //   log.error("get socket flags failed");
  //   throw std::runtime_error("get socket flags failed");
  // }
  //
  // // set to blocking for ssl handshake
  // if (fcntl(this->socket, F_SETFL, flag & ~O_NONBLOCK) == -1)
  // {
  //   SSL_free(this->ssl_);
  //   log.error("Failed to set socket to blocking mode");
  //   throw std::runtime_error("Failed to set socket to blocking mode");
  // }
  //
  // log.debug("accepting ssl connection...");
  // int err = SSL_accept(this->ssl_);
  // if (err <= 0)
  // {
  //   err = SSL_get_error(this->ssl_, err);
  //   if (err != SSL_ERROR_NONE)
  //   {
  //     SSL_free(this->ssl_);
  //     log.error("ssl accept failed");
  //     throw std::runtime_error("ssl accept failed");
  //   }
  // }
  //
  // if (fcntl(this->socket, F_SETFL, flag))
  // {
  //   SSL_free(this->ssl_);
  //   log.error("Failed to restore socket flags after TLS handshake");
  //   throw std::runtime_error("Failed to restore socket flags after TLS handshake");
  // }
  //
  // log.info("ssl handshake succeded");

  // TODO should remove the blocking bs.
  // relay on epoll for multy step handshake
}

TlsServer::Client::~Client()
{
  SSL_shutdown(this->ssl_);
  SSL_free(this->ssl_);
  Logger::info("ssl shutdown");
}

ssize_t TlsServer::Client::recv(int __fd, void* __buf, size_t __n, int __flags)
{
  ssize_t bytes = SSL_read(this->ssl_, __buf, __n);
  return bytes;
}

ssize_t TlsServer::Client::send(int __fd, const void* __buf, size_t __n, int __flags)
{
  ssize_t bytes = SSL_write(this->ssl_, __buf, __n);
  return bytes;
}
