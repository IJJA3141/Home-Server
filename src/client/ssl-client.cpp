#include "./client.hpp"

http::SSLClient::SSLClient(SSL_CTX *_pCTX, const int *_pSocket)
    : http::Client(_pSocket) {
  this->type = "Ssl client";
  this->pSSL_ = SSL_new(_pCTX);
  SSL_set_fd(this->pSSL_, this->socket_);

  if (SSL_accept(this->pSSL_) == -1) {
    std::cerr << "Secured connection with the client failed." << std::endl;
    ERR_print_errors_fp(stderr);
    this->state = false;
  };

  return;
}

size_t http::SSLClient::Read() {
  return SSL_read(this->pSSL_, this->buffer, this->bufferSize_);
};

int http::SSLClient::Send(const char *_pBuffer) {
  return SSL_write(this->pSSL_, _pBuffer, strlen(_pBuffer));
}

http::SSLClient::~SSLClient() {
  SSL_shutdown(this->pSSL_);
  SSL_free(this->pSSL_);

  return;
}
