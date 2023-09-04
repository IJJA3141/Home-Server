#include "./client.hpp"
#include <cstdio>
#include <iostream>
#include <openssl/err.h>
#include <openssl/ssl.h>

http::SSLClient::SSLClient(SSL_CTX *_pCTX, const int *_pSocket)
    : http::Client(_pSocket) {
  this->pSSL_ = SSL_new(_pCTX);
  SSL_set_fd(this->pSSL_, this->socket);

  if (SSL_accept(this->pSSL_) == -1) {
    std::cerr << "Secured connection with the client failed." << std::endl;
    ERR_print_errors_fp(stderr);
    // abort();
  };

  this->type = "ssl client";

  return;
}
