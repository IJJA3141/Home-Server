#include "client.hpp"
#include "../log.hpp"
#include "reqres.hpp"
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <unistd.h>

Client::Client(const int &_socket)
{
  this->socket_size_ = sizeof(this->client_);
  this->buffer_size_ = sizeof(this->buffer_);
  this->socket_ = accept(_socket, (struct sockaddr *)&this->client_, &this->socket_size_);

  if (this->socket_ == -1) VERBERR("connection with the client failed.");

  return;
}

Client::~Client()
{
  ::close(this->socket_);

  return;
}

SSLClient::SSLClient(const int &_socket, SSL_CTX *_ctx) : Client(_socket)
{
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

Request Client::read()
{
  size_t bytes = ::read(this->socket_, this->buffer_, this->buffer_size_);
  if (bytes < 0) {
    VERBERR("failed to read client's message.")
    return Request();
  }

  LOG("reading client message.");
  this->buffer_[bytes] = '\0';

  return Request(this->buffer_, this->type_);
}

void Client::send(const Response _res) const
{
  std::string res = "";

  res += _res.cmd.protocol + " " + std::to_string(_res.cmd.status_code) + "\n";
  res += "content-length: " + std::to_string(_res.body.size()) + "\n";

  for (const auto &header : _res.headers)
    res += header.first + ": " + header.second + "\n";

  res += "\n" + _res.body;

  if (::write(this->socket_, res.c_str(), res.size() < 0)) {
    ERR("?");
  };
}
