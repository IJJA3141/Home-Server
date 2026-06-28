#include "../common/logger.hpp"
#include "reverse_proxy.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>

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

TlsServer::TlsServer(const std::string& ip, uint16_t port, const std::filesystem::path& cert,
                     const std::filesystem::path& key, protocol::Handler<ForwardPolicy> handler)
    : TcpServer(ip, port, handler)
{
  auto log = Logger::get("TSL Server", [] { return ERR_error_string(ERR_get_error(), nullptr); });

  load_lib_ssl();

  if ((this->ctx_ = SSL_CTX_new(TLS_method())) == nullptr)
  {
    unload_lib_ssl();
    log.crit("SSL context creation failed");
    throw std::runtime_error("SSL context creation failed");
  }
  log.debug("new ssl ctx created");

  if (SSL_CTX_use_certificate_file(this->ctx_, cert.c_str(), SSL_FILETYPE_PEM) != 1)
  {
    unload_lib_ssl();
    SSL_CTX_free(this->ctx_);
    log.crit("ssl certificate loading {} failed", cert);
    throw std::runtime_error("ssl certificate loading failed");
  }
  log.debug("certificate loaded");

  if (SSL_CTX_use_PrivateKey_file(this->ctx_, key.c_str(), SSL_FILETYPE_PEM) != 1)
  {
    unload_lib_ssl();
    SSL_CTX_free(this->ctx_);
    log.crit("ssl key {} loading failed", key);
    throw std::runtime_error("ssl key loading failed");
  }
  log.debug("key loaded");

  if (SSL_CTX_check_private_key(this->ctx_) != 1)
  {
    unload_lib_ssl();
    SSL_CTX_free(this->ctx_);
    log.crit("certificate {} doesn't match key {}", cert, key);
    throw std::runtime_error("certificate and key don't match");
  }
  log.debug("certificate and key match");
  log.info("created...");
}

TlsServer::TlsServer::~TlsServer()
{
  auto log = Logger::get("TLS Server");
  SSL_CTX_free(this->ctx_);
  unload_lib_ssl();
  log.debug("deleted...");
}

void TlsServer::accept()
{
  auto log = Logger::get("TLS Server");

  log.info("accepting new client connection");
  try
  {
    new TlsServer::Client(this->listening_socket_, this->epoll_fd_, this->ctx_, this->handler_);
  }
  catch (const std::system_error& e)
  { // client connection failed
    log.warn("client connection failed (error={})", e.what());
  }
  catch (const std::runtime_error& e)
  { // client handshake failed
    log.warn("client handshake failed (error={})", e.what());
  }
}
