#pragma once

#include "http.hpp"

#include <netinet/in.h>
#include <openssl/crypto.h>

class Client {
public:
  Client(const int &_socket);
  /*shuts down the client*/
  ~Client();

  virtual http::Request read();
  virtual void send(http::Response _response) const;

protected:
  int socket_;
  sockaddr_in client_;
  socklen_t socket_size_;
  size_t buffer_size_;
  char buffer_[4096];
  bool secured_;

  virtual size_t socket_read();
  virtual void socket_write(const std::string _res) const;
};

class SSLClient : public Client {
public:
  SSLClient(const int &_socket, SSL_CTX *_ctx);
  ~SSLClient();

private:
  ::SSL *ssl_;

  size_t socket_read() override;
  void socket_write(const std::string _res) const override;
};
