#include "../log.hpp"
#include "server.hpp"

#include <openssl/ssl.h>

#define ERR_CTX      "Failed to create SSL context with TLS_method()"
#define ERR_CERT     "Failed to load SSL certificate file"
#define ERR_KEY      "Failed to load SSL private key file"
#define ERR_MISMATCH "SSL private key does not match certificate"

bool Tls::SSL_LIB_INIT_ = false;
size_t Tls::ATOMIC_COUNT_ = 0;

Tls::Tls(const int _port, const std::filesystem::path& _cert, const std::filesystem::path& _key,
         const Router& _router)
    : Tcp(_port, _router)
{
  if (!Tls::SSL_LIB_INIT_)
  {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    Tls::SSL_LIB_INIT_ = true;
  }

  assert((this->ctx_ = SSL_CTX_new(TLS_method())) != NULL, ERR_CTX);
  assert(SSL_CTX_use_certificate_file(this->ctx_, _cert.c_str(), SSL_FILETYPE_PEM) > 0, ERR_CERT);
  assert(SSL_CTX_use_PrivateKey_file(this->ctx_, _key.c_str(), SSL_FILETYPE_PEM) > 0, ERR_KEY);
  assert(SSL_CTX_check_private_key(this->ctx_), ERR_MISMATCH);

  Tls::ATOMIC_COUNT_++;
  return;
}

Tls::~Tls()
{
  if (--Tls::ATOMIC_COUNT_ == 0)
  {
    OPENSSL_cleanup();
    Tls::SSL_LIB_INIT_ = false;
  }

  return;
}

Client* Tls::anchor_client() const
{
  return new SSL_Client(this->epoll_, this->socket_, this->addr_, this->ctx_);
};
