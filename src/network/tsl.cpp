#include "../log.hpp"
#include "server.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>

Tls::Tls(const size_t _client_size, const Router *_router, const std::string _cert_path, const std::string _key_path) : Tcp(_router)
{
  LOG("ssl initialization...");

  if (!SSLLIBINIT) {
    LOG("loading OpenSSL library...");
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSLLIBINIT = true;
  }

  this->ctx_ = SSL_CTX_new(TLS_method());
  if (this->ctx_ == NULL) {
    VERBERR("loading SSL method failed.");
    exit(1);
  }

  LOG("loading certificate...");
  if (SSL_CTX_use_certificate_file(this->ctx_, _cert_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
    VERBERR("loading certificate file failed.");
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  if (SSL_CTX_use_PrivateKey_file(this->ctx_, _key_path.c_str(), SSL_FILETYPE_PEM) <= 0) {
    VERBERR("loading certificate key failed.");
    ERR_print_errors_fp(stderr);
    exit(-1);
  }

  if (!SSL_CTX_check_private_key(this->ctx_)) {
    VERBERR("private key does not match the pulic certificate.");
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  return;
}

Client *Tls::await_client() { return new SSLClient(this->socket_, this->ctx_); }

Tls::~Tls()
{
  OPENSSL_cleanup();
  return;
}
