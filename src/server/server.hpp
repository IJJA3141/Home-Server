#pragma once

#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <string>
#include <thread>
#include <vector>

// http
class Client
{
public:
  char buffer[4096];

  Client(const int &_socket);
  ~Client();

  virtual size_t read();
  virtual int send(const std::string _message);

protected:
  int socket_;
  sockaddr_in client_;
  socklen_t size_;
  size_t bufferSize_;

  enum Type { NONSSL, SSL };
  const Type type = NONSSL;
};

class TcpServer
{
public:
  std::vector<std::thread *> threads;

  TcpServer();
  ~TcpServer();

  void connect(Client *_client);
  void bind(const int _port);
  void listen();

protected:
  int socket_;
  struct sockaddr_in hint_;
  int port_;

  virtual Client *newClient_();
};

// https
class SSLClient : public Client
{
public:
  SSLClient(const int &_socket, SSL_CTX *_CTX);
  ~SSLClient();

  size_t read() override;
  int send(const std::string _message) override;

private:
  ::SSL *ssl_;
  const Type type = SSL;
};

static bool SSLLIBINIT = false;

class TlsServer : private TcpServer
{
public:
  TlsServer(const char *_certFile, const char *_keyFile);
  ~TlsServer();

private:
  SSL_CTX *CTX_;

  virtual Client *newClient_() override;
};
