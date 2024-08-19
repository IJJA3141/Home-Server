#include <openssl/err.h>

#include "../log.hpp"
#include "server.hpp"

Tls::Tls(const Router *_parser, const char *_certFile, const char *_keyFile) : Tcp(_parser)
{
  LOG("ssl initialization...");

  if (!SSLLIBINIT) {
    LOG("loading OpenSSL library...");
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSLLIBINIT = true;
  }

  this->CTX_ = SSL_CTX_new(TLS_method());
  if (this->CTX_ == NULL) {
    VERBERR("loading SSL method failed.");
    exit(1);
  }

  LOG("loading certificate...");
  if (SSL_CTX_use_certificate_file(this->CTX_, _certFile, SSL_FILETYPE_PEM) <= 0) {
    VERBERR("loading certificate file failed.");
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  if (SSL_CTX_use_PrivateKey_file(this->CTX_, _keyFile, SSL_FILETYPE_PEM) <= 0) {
    VERBERR("loading certificate key failed.");
    ERR_print_errors_fp(stderr);
    exit(-1);
  }

  if (!SSL_CTX_check_private_key(this->CTX_)) {
    VERBERR("private key does not match the pulic certificate.");
    ERR_print_errors_fp(stderr);
    exit(1);
  }

  return;
}

Client *Tls::newClient_() { return new SSLClient(this->socket_, this->CTX_); }

Tls::~Tls()
{
  OPENSSL_cleanup();
  return;
}
