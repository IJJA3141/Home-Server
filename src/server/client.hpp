#pragma once

#include <netinet/in.h>
#include <openssl/crypto.h>

struct Request;
struct Response;

class Client
{
public:
  enum Type { STANDARD, SSL };

  Client(const int &_socket);
  ~Client();

  virtual Request read() ;
  virtual void send(const Response _res) const;

protected:
  const Type type_ = STANDARD;
  int socket_;
  sockaddr_in client_;
  socklen_t socket_size_;
  size_t buffer_size_;
  char buffer_[4096];
};

class SSLClient : public Client
{
public:
  SSLClient(const int &_socket, SSL_CTX *_ctx);
  ~SSLClient();

  Request read() override;
  void send(const Response _res) const override;

private:
  const Type type_ = SSL;
  ::SSL *ssl_;
};
