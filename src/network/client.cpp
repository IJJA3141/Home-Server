#include "client.hpp"
#include "../log.hpp"
#include "http.hpp"

#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#define WARN_SERVERR "Server encountered an error while responding to client."

#define ERR_READ             "Failed to read data from client socket."
#define ERR_WRITE            "Failed to send data to client socket."
#define ERR_ACCEPT_CLIENT    "Failed to accept new client connection."
#define ERR_GET_IP           "Failed to get client ip addresse."
#define ERR_EPOLL_ADD_CLIENT "Failed to register client socket with epoll instance."
#define ERR_SSL_CREATE       "Failed to initialize SSL session for client."
#define ERR_SSL_FD_BIND      "Failed to bind SSL session to socket descriptor."
#define ERR_EPOLL_CREATE     "Failed to create epoll instance for SSL handshake."
#define ERR_EPOLL_ADD_SSL    "Failed to register SSL socket with epoll for handshake readiness."
#define ERR_EPOLL_CLOSE      "Failed to gracefully close epoll file descriptor."
#define ERR_SSL_HANDSHAKE    "SSL handshake with client failed."
#define ERR_SSL_SHUTDOWN     "SSL shutdown procedure did not complete successfully."
#define ERR_EPOLL_WAIT       "epoll_wait failed while waiting for SSL handshake readiness."
#define ASS_SOCKET_CLOSE     "Unable to close client socket."
#define ASS_EPOLL_REMOVE     "Unable to deregister client socket from epoll instance."

#define BUFFER_SIZE 4096
#define READ_FAILED                                                                                               \
  {                                                                                                               \
    err(ERR_READ);                                                                                                \
    return {http::Error::READ};                                                                                   \
  }

// iclient
http::Request Client::read() const
{
  char buffer[BUFFER_SIZE];
  std::string message;
  ssize_t bytes;

  while ((bytes = this->recv(buffer)) == BUFFER_SIZE)
  {
    if (bytes < 0) READ_FAILED
    buffer[bytes] = '\0';
    message += buffer;
  }

  if (bytes < 0) READ_FAILED
  buffer[bytes] = '\0';
  message += buffer;

  return http::parse_request(message);
}

void Client::write(http::Response _response) const
{
  check(Level::WARN, _response.status < 500, WARN_SERVERR);
  check(Level::ERR, this->send(_response) >= 0, ERR_WRITE);

  return;
}

// tcp
Client::Client(int _epoll, int _socket, sockaddr_in _addr) : epoll_(_epoll), moored(true)
{
  socklen_t len = sizeof _addr;
  this->socket_ = accept4(_socket, reinterpret_cast<sockaddr*>(&_addr), &len, SOCK_NONBLOCK);
  if (this->socket_ == -1)
  {
    err(ERR_ACCEPT_CLIENT);
    this->moored = false;
    return;
  }

  sockaddr_storage addr;
  len = sizeof addr;
  if (getpeername(this->socket_, reinterpret_cast<sockaddr*>(&addr), &len) == -1)
  {
    err(ERR_GET_IP);
    assert(::close(this->socket_) != -1, ASS_SOCKET_CLOSE);
    this->moored = false;
    return;
  }

  sockaddr_in* s = reinterpret_cast<sockaddr_in*>(&addr);
  int port = ntohs(s->sin_port);
  inet_ntop(AF_INET, &s->sin_addr, this->ip, sizeof this->ip);

  epoll_event event(EPOLLIN | EPOLLET | EPOLLRDHUP, epoll_data(this));
  if (epoll_ctl(_epoll, EPOLL_CTL_ADD, this->socket_, &event) == -1)
  {
    err(ERR_EPOLL_ADD_CLIENT);
    assert(::close(this->socket_) != -1, ASS_SOCKET_CLOSE);
    this->moored = false;
    return;
  }

  return;
}

Client::~Client()
{
  if (this->moored) this->close();
}

void Client::close() const
{
  assert(epoll_ctl(this->epoll_, EPOLL_CTL_DEL, this->socket_, nullptr) != -1, ASS_EPOLL_REMOVE);
  assert(::close(this->socket_) != -1, ASS_SOCKET_CLOSE);

  return;
}

ssize_t Client::recv(char* const _buffer) const { return ::recv(this->socket_, _buffer, BUFFER_SIZE, 0); }
int Client::send(const std::string& _msg) const { return ::send(this->socket_, _msg.c_str(), _msg.size(), 0); }

// tls
SSL_Client::SSL_Client(const int _epoll, const int _socket, const sockaddr_in _addr, SSL_CTX* const _ctx)
    : Client(_epoll, _socket, _addr)
{
  if (!this->moored) return;

  if ((this->ssl_ = SSL_new(_ctx)) == NULL)
  {
    err(ERR_SSL_CREATE);
    this->close();
    this->moored = false;

    return;
  }

  if (SSL_set_fd(this->ssl_, this->socket_) == -1)
  {
    err(ERR_SSL_FD_BIND);
    SSL_free(this->ssl_);
    this->close();
    this->moored = false;

    return;
  };

  if (this->dehaling())
  {
    err(ERR_SSL_HANDSHAKE);
    SSL_free(this->ssl_);
    this->close();
    this->moored = false;
  }

  return;
};

SSL_Client::~SSL_Client()
{
  if (this->moored)
  {
    assert(SSL_shutdown(this->ssl_) != -1, ERR_SSL_SHUTDOWN);
    SSL_free(this->ssl_);
  }

  return;
}

// true if failed
bool SSL_Client::dehaling() const
{
  epoll_event event(EPOLLIN | EPOLLOUT, epoll_data{});
  int epoll, res;

  if ((epoll = epoll_create1(0)) == -1)
  {
    err(ERR_EPOLL_CREATE);
    return true;
  };

  if (epoll_ctl(epoll, EPOLL_CTL_ADD, this->socket_, &event) == -1)
  {
    err(ERR_EPOLL_ADD_SSL);
    assert(::close(epoll) != -1);
    return true;
  };

  res = SSL_get_error(this->ssl_, SSL_accept(this->ssl_));
  while (res == SSL_ERROR_WANT_READ || res == SSL_ERROR_WANT_WRITE)
  {
    if (epoll_wait(epoll, &event, 1, -1) == -1)
    {
      err(ERR_EPOLL_WAIT);
      assert(::close(epoll) != -1, ERR_EPOLL_CLOSE);
      return true;
    }

    res = SSL_get_error(this->ssl_, SSL_accept(this->ssl_));
  }

  assert(::close(epoll) != -1, ERR_EPOLL_CLOSE);
  return res != SSL_ERROR_NONE;
}

ssize_t SSL_Client::recv(char* const _buffer) const { return SSL_read(this->ssl_, _buffer, BUFFER_SIZE); }
int SSL_Client::send(const std::string& _msg) const { return SSL_write(this->ssl_, _msg.c_str(), _msg.size()); }
