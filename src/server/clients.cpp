#include <openssl/err.h>
#include <unistd.h>

#include "../log.hpp"
#include "server.hpp"

Client::Client(const int &_socket)
{
  this->size_ = sizeof(this->client_);
  this->bufferSize_ = sizeof(this->buffer);
  this->socket_ = accept(_socket, (struct sockaddr *)&this->client_, &this->size_);

  if (this->socket_ == -1) {
    VERBERR("connection with the client failed.");
  }

  switch (this->type) {
  case NONSSL:
    LOG("new nonssl client connected.");
    break;
  case SSL:
    LOG("new ssl client connected.");
    break;
  }

  return;
}

size_t Client::read() { return ::read(this->socket_, this->buffer, this->bufferSize_); }

int Client::send(const std::string _message)
{
  return ::write(this->socket_, _message.c_str(), _message.size());
}

Client::~Client()
{
  switch (this->type) {
  case NONSSL:
    LOG("nonssl client closed.");
    break;
  case SSL:
    LOG("ssl client closed.");
    break;
  }

  ::close(this->socket_);

  return;
}

SSLClient::SSLClient(const int &_socket, SSL_CTX *_CTX) : Client(_socket)
{
  this->ssl_ = SSL_new(_CTX);
  SSL_set_fd(this->ssl_, this->socket_);

  if (SSL_accept(this->ssl_) == -1) {
    VERBERR("secured connection with the client failed.");
    ERR_print_errors_fp(stderr);
  }

  return;
};

size_t SSLClient::read() { return SSL_read(this->ssl_, this->buffer, this->bufferSize_); }

int SSLClient::send(const std::string _message)
{
  return SSL_write(this->ssl_, _message.c_str(), _message.size());
}

SSLClient::~SSLClient()
{
  SSL_shutdown(this->ssl_);
  SSL_free(this->ssl_);

  return;
}
