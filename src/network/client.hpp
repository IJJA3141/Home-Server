#pragma once

#include "http.hpp"

#include <netinet/in.h>
#include <openssl/crypto.h>

#define CLIENT_BUFF_SIZE 4096

class Client
{
public:
  Client(const int _socket, const int _epoll);
  ~Client();

  Request read();
  void send(const Response _response);

  inline operator int() const { return this->socket_; }

protected:
  virtual inline ssize_t socket_read();
  virtual inline void socket_write(const std::string& _msg);

protected:
  int socket_;
  int epoll_;

  sockaddr_in client_;
  socklen_t socket_size_;
  size_t buffer_size_;
  char buffer_[CLIENT_BUFF_SIZE];
};

class SSL_Client : public Client
{
public:
  SSL_Client(const int _socket, const int _epoll, SSL_CTX* _ctx);
  ~SSL_Client();

private:
  ::SSL* ssl_;

  inline ssize_t socket_read() override;
  inline void socket_write(const std::string& _msg) override;
};
