#include "../log.hpp"
#include "server.hpp"

#include <openssl/ssl.h>

bool Tls::SSLLIBINIT = false;

Tls::Tls(int _port, const std::filesystem::path& _cert, const std::filesystem::path& _key, const Router& _router)
    : Tcp(_port, _router)
{
  if (!Tls::SSLLIBINIT)
  {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    Tls::SSLLIBINIT = true;
  }

  // assert(this->ctx_ != NULL);
  assert((this->ctx_ = SSL_CTX_new(TLS_method())) != nullptr);
  assert(SSL_CTX_use_certificate_file(this->ctx_, _cert.c_str(), SSL_FILETYPE_PEM) > 0);
  assert(SSL_CTX_use_PrivateKey_file(this->ctx_, _key.c_str(), SSL_FILETYPE_PEM) > 0);
  assert(SSL_CTX_check_private_key(this->ctx_));

  return;
}

Tls::~Tls() { OPENSSL_cleanup(); }
Client* Tls::anchor_client() const { return new SSL_Client(this->epoll_, this->socket_, this->addr_, this->ctx_); };
