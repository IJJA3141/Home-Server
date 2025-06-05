#include "client.hpp"
#include "../log.hpp"

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <string>
#include <unistd.h>

Client::Client(const int &_socket) : secured_(false)
{
  INFO("new client ");

  this->socket_size_ = sizeof(this->client_);
  this->buffer_size_ = sizeof(this->buffer_);
  this->socket_ = accept(_socket, (struct sockaddr *)&this->client_, &this->socket_size_);

  if (this->socket_ == -1) VERBERR("connection with the client failed.");

  return;
}

Client::~Client() { ::close(this->socket_); }

http::Request Client::read()
{
  size_t bytes = this->socket_read();
  if (bytes < 0) {
    VERBERR("failed to read client's message.")
    return http::Request(http::Request::Failure::MALFORMED);
  }

  LOG("reading client message.");
  this->buffer_[bytes] = '\0';

  LOG("reading\n" << this->buffer_);
  return http::Request(this->buffer_, this->secured_);
}

void Client::send(const http::Response _res) const
{
  if (_res.cmd.status_code != 200) {
    WARN("sending\n" + std::string(_res));
  } else {
    LOG("sending\n" + std::string(_res));
  }

  this->socket_write(_res);
  return;
}

size_t Client::socket_read() { return ::read(this->socket_, this->buffer_, this->buffer_size_); }

void Client::socket_write(const std::string _res) const
{
  if (::write(this->socket_, _res.c_str(), _res.size()) < 0) {
    ERR("failed to write to the client !?");
  };

  return;
}

SSLClient::SSLClient(const int &_socket, SSL_CTX *_ctx) : Client(_socket)
{
  this->secured_ = true;

  this->ssl_ = SSL_new(_ctx);
  SSL_set_fd(this->ssl_, this->socket_);

  if (SSL_accept(this->ssl_) == -1) {
    VERBERR("secured connection with the client failed.");
    ERR_print_errors_fp(stderr);
  }

  return;
};

SSLClient::~SSLClient()
{
  SSL_shutdown(this->ssl_);
  SSL_free(this->ssl_);

  return;
}

size_t SSLClient::socket_read() { return SSL_read(this->ssl_, this->buffer_, this->buffer_size_); }

void SSLClient::socket_write(const std::string _res) const
{
  if (SSL_write(this->ssl_, _res.c_str(), _res.size()) < 0)
    ERR("failed to write to the ssl client !?");

  return;
}
