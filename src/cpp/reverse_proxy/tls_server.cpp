#include "server.hpp"
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

TlsServer::TlsServer(const std::string& _ip, uint16_t _port, Epoll _epoll, ForwardingTable _fw_tbl,
                     const std::filesystem::path& _cert, const std::filesystem::path& _key)
    : TcpServer{_ip, _port, _epoll, _fw_tbl}
{
  load_lib_ssl();

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

void TlsServer::notify_read()
{
  try
  {
    new TlsServer::Client(this->listening_socket_, *this);
  }
  catch (const std::exception& e)
  {
    const auto& log = Logger::get(std::format("TLS Server"), [] { return strerror(errno); });
    log.error("client threw an error while registering error: {}", e.what());
  }
}
