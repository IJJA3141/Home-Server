#include "../log.hpp"
#include "server.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

bool Tls::SSLLIBINIT = false;

Tls::Tls(const size_t _pool_size, const std::filesystem::path& _cert, const std::filesystem::path& _key)
    : Tcp(_pool_size)
{
  if (!Tls::SSLLIBINIT)
  {
    // log("loading OpenSSL library...");
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    Tls::SSLLIBINIT = true;
  }

  this->ctx_ = SSL_CTX_new(TLS_method());
  assert(this->ctx_ != NULL);

  // log("loading certificate...");
  assert(SSL_CTX_use_certificate_file(this->ctx_, _cert.c_str(), SSL_FILETYPE_PEM) > 0);
  assert(SSL_CTX_use_PrivateKey_file(this->ctx_, _key.c_str(), SSL_FILETYPE_PEM) > 0);
  assert(SSL_CTX_check_private_key(this->ctx_));

  return;
}

Tls::~Tls()
{
  OPENSSL_cleanup();
  return;
}

std::unique_ptr<Client> Tls::await_client()
{
  return std::make_unique<SSL_Client>(this->socket_, this->epoll_, this->ctx_);
}
