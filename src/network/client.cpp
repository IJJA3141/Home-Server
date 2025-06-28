#include "client.hpp"
#include "../log.hpp"
#include "http.hpp"

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <unistd.h>

// common
Request Client::read()
{
  ssize_t bytes = this->socket_read();
  if (bytes < 0)
  {
    err("client failed reading");
    return {Request::READ};
  };

  if (bytes == 0)
  {
    warn("client removed");
    return {Request::CLOSED};
  }

  this->buffer_[bytes] = '\0';

  std::string str = this->buffer_;

  debug(str);

  if (bytes == CLIENT_BUFF_SIZE)
  {
    while ((bytes = this->socket_read()) == CLIENT_BUFF_SIZE)
    {
      err("fdp");
      str += this->buffer_;
    }

    this->buffer_[bytes] = '\0';
    str += this->buffer_;
  }

  log("new message from client");
  debug(str);
  return parse_request(str);
}

void Client::send(const Response _response)
{
  // if (_response.code != 200) warn("sending\n" + (std::string)_response);

  this->socket_write(_response);
  return;
}

// client specifications
Client::Client(const int _socket, const int _epoll) : socket_size_(sizeof(this->client_))
{
  this->socket_ = accept(_socket, (struct sockaddr*)&this->client_, &this->socket_size_);
  assert(this->socket_ != -1);

  epoll_event event;
  event.events = EPOLLIN | EPOLLONESHOT;
  event.data.fd = _socket;

  assert(epoll_ctl(_epoll, EPOLL_CTL_ADD, _socket, &event) != -1, "epoll failed", errno, AT);

  log("new client created");
  return;
}

Client::~Client()
{
  close(this->socket_);
  assert(epoll_ctl(this->epoll_, EPOLL_CTL_DEL, this->socket_, nullptr) != -1);

  return;
}

ssize_t Client::socket_read() { return ::read(this->socket_, this->buffer_, this->buffer_size_); }

void Client::socket_write(const std::string& _res)
{
  if (::write(this->socket_, _res.c_str(), _res.size()) < 0)
    ; // err("failed to write to the client !?");
}

// ssl client specifications
SSL_Client::SSL_Client(const int _socket, const int _epoll, SSL_CTX* _ctx) : Client(_socket, _epoll)
{
  this->ssl_ = SSL_new(_ctx);
  SSL_set_fd(this->ssl_, this->socket_);

  assert(SSL_accept(this->ssl_) != -1);

  return;
};

SSL_Client::~SSL_Client()
{
  SSL_shutdown(this->ssl_);
  SSL_free(this->ssl_);

  return;
}

ssize_t SSL_Client::socket_read() { return SSL_read(this->ssl_, this->buffer_, this->buffer_size_); }

void SSL_Client::socket_write(const std::string& _res)
{
  if (SSL_write(this->ssl_, _res.c_str(), _res.size()) < 0)
    ; // err("failed to write to the ssl client !?");
}
