#include "client.hpp"
#include "../log.hpp"
#include "http.hpp"

#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

// iclient
http::Request Client::read() const
{
  char buffer[BUFFER_SIZE];
  std::string message;
  ssize_t bytes;

  while ((bytes = this->recv(buffer)) == BUFFER_SIZE)
  {
    if (!check(Level::ERR, bytes >= 0, "read failed")) return {http::Error::READ};

    buffer[bytes] = '\0';
    message += buffer;
  }

  if (!check(Level::ERR, bytes >= 0, "read failed")) return {http::Error::READ};

  buffer[bytes] = '\0';
  message += buffer;

  debug((std::string)http::parse_request(message));
  return http::parse_request(message);
}

void Client::write(http::Response _response) const
{
  _response.headers["Content-Length"] = std::format("{}", _response.body.size() + 2);

  check(Level::WARN, _response.status < 500 || 599 < _response.status, "error while responding to client");
  check(Level::ERR, this->send(_response) >= 0, "");

  return;
}

// tcp
Client::Client(int _epoll, int _socket, sockaddr_in _addr) : epoll_(_epoll), init(true)
{
  socklen_t len = sizeof(_addr);
  this->socket_ = accept4(_socket, reinterpret_cast<sockaddr*>(&_addr), &len, SOCK_NONBLOCK);
  if (this->socket_ == -1)
  {
    err("accept");
    this->init = false;
    return;
  }

  epoll_event event(EPOLLIN | EPOLLET | EPOLLRDHUP, epoll_data(this));
  if (epoll_ctl(_epoll, EPOLL_CTL_ADD, this->socket_, &event) == -1)
  {
    err("epoll");
    assert(::close(this->socket_) != -1);
    this->init = false;
    return;
  }

  return;
}

Client::~Client()
{
  if (this->init) this->close();

  return;
}

void Client::close()
{
  assert(epoll_ctl(this->epoll_, EPOLL_CTL_DEL, this->socket_, nullptr) != -1);
  assert(::close(this->socket_) != -1);

  return;
}

ssize_t Client::recv(char* _buffer) const { return ::recv(this->socket_, _buffer, BUFFER_SIZE, 0); }
int Client::send(const std::string& _msg) const { return ::send(this->socket_, _msg.c_str(), _msg.size(), 0); }

// tls
SSL_Client::SSL_Client(int _epoll, int _socket, sockaddr_in _addr, SSL_CTX* _ctx) : Client(_epoll, _socket, _addr)
{
  if (!this->init) return;

  if ((this->ssl_ = SSL_new(_ctx)) == NULL)
  {
    err("ssl new");
    this->close();
    this->init = false;

    return;
  }

  if (SSL_set_fd(this->ssl_, this->socket_) == -1)
  {
    err("set fd");
    SSL_free(this->ssl_);
    this->close();
    this->init = false;

    return;
  };

  epoll_event event(EPOLLIN | EPOLLOUT, epoll_data{});
  int epoll, res;

  if ((epoll = epoll_create1(0)) == -1)
  {
    err("epoll");
    SSL_free(this->ssl_);
    this->close();
    this->init = false;

    return;
  };

  if (epoll_ctl(epoll, EPOLL_CTL_ADD, this->socket_, &event) == -1)
  {
    err("epoll add");
    ::close(epoll);
    SSL_free(this->ssl_);
    this->close();
    this->init = false;

    return;
  };

  res = SSL_get_error(this->ssl_, SSL_accept(this->ssl_));
  while (res == SSL_ERROR_WANT_READ || res == SSL_ERROR_WANT_WRITE)
  {
    if (epoll_wait(epoll, &event, 1, -1) == -1)
    {
      err("epoll wait");
      ::close(epoll);
      SSL_free(this->ssl_);
      this->close();
      this->init = false;

      return;
    }

    res = SSL_get_error(this->ssl_, SSL_accept(this->ssl_));
  }

  if (res != SSL_ERROR_NONE)
  {
    ::close(epoll);
    err("ssl accept");
    SSL_free(this->ssl_);
    this->close();
    this->init = false;

    return;
  }

  ::close(epoll);
  return;
};

SSL_Client::~SSL_Client()
{
  if (this->init)
  {
    SSL_shutdown(this->ssl_);
    SSL_free(this->ssl_);
  }

  return;
}

ssize_t SSL_Client::recv(char* _buffer) const { return SSL_read(this->ssl_, _buffer, BUFFER_SIZE); }
int SSL_Client::send(const std::string& _res) const { return SSL_write(this->ssl_, _res.c_str(), _res.size()); }
