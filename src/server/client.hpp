#pragma once

#include <netinet/in.h>
#include <openssl/crypto.h>
#include <string>

struct Request;
struct Response;

class Client
{
public:
  enum Type { STANDARD, SSL };

  Client(const int &_socket);
  ~Client();

  virtual Request read();
  virtual void send(const Response _res) const;

protected:
  const Type type_ = STANDARD;
  int socket_;
  sockaddr_in client_;
  socklen_t socket_size_;
  size_t buffer_size_;
  char buffer_[4096];

  virtual size_t socket_read();
  virtual void socket_write(const std::string _res) const;
};

class SSLClient : public Client
{
public:
  SSLClient(const int &_socket, SSL_CTX *_ctx);
  ~SSLClient();

private:
  const Type type_ = SSL;
  ::SSL *ssl_;

  size_t socket_read() override;
  void socket_write(const std::string _res) const override;
};
