#pragma once

#include "http.hpp"

#include <netinet/in.h>
#include <openssl/crypto.h>

#define CLIENT_BUFF_SIZE 4096

class Client
{
public:
  Client(int _epoll, int _socket, sockaddr_in _addr);
  ~Client();

  http::Request read() const;
  void write(http::Response _response) const;

protected:
  int socket_;

private:
  const int epoll_;

  virtual ssize_t recv(char* _buffer) const;
  virtual int send(const std::string& _message) const;
};

class SSL_Client : public Client
{
public:
  SSL_Client(int _epoll, int _socket, sockaddr_in _addr, SSL_CTX* _ctx);
  ~SSL_Client();

private:
  ::SSL* ssl_;

  ssize_t recv(char* _buffer) const override;
  int send(const std::string& _message) const override;
};
