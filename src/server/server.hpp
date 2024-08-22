#pragma once

#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <thread>
#include <vector>

class Router;

struct Response{};

// http
class Client
{
public:
  enum Type { NONSSL = 0, SSL = 2 };
  const Type type = NONSSL;
  char buffer[4096];

  Client(const int &_socket);
  ~Client();

  virtual Request read();
  virtual int send(const Response _res);

protected:
  int socket_;
  sockaddr_in client_;
  socklen_t size_;
  size_t bufferSize_;
};

class Tcp
{
public:
  std::vector<std::thread *> threads;

  Tcp(const Router *_parser);
  ~Tcp();

  void connect(Client *_client);
  void bind(const int _port);
  void listen();

protected:
  int socket_;
  struct sockaddr_in hint_;
  int port_;
  const Router *router_;

  virtual Client *newClient_();
};

// https
class SSLClient : public Client
{
public:
  const Type type = SSL;

  SSLClient(const int &_socket, SSL_CTX *_CTX);
  ~SSLClient();

  Request read() override;
  int send(const Response _res) override;

private:
  ::SSL *ssl_;
};

static bool SSLLIBINIT = false;

class Tls : private Tcp
{
public:
  Tls(const Router *_parser, const char *_certFile, const char *_keyFile);
  ~Tls();

private:
  SSL_CTX *CTX_;

  virtual Client *newClient_() override;
};
