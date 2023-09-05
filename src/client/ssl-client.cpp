#include "./client.hpp"
#include <cstdio>
#include <iostream>
#include <iterator>
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

size_t http::SSLClient::Read() {
  std::cout << "reading from ssl client" << std::endl;
  return SSL_read(this->pSSL_, this->buffer, this->bufferSize_);
};

int http::SSLClient::Send(const char *_pBuffer) {
  return SSL_write(this->pSSL_, _pBuffer, strlen(_pBuffer));
}
