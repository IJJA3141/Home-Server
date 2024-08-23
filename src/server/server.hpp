#pragma once

#include "client.hpp"
#include "router.hpp"
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <thread>
#include <vector>

class Tcp
{
public:
  std::vector<std::thread *> active_client;

  Tcp(const Router *_router);
  ~Tcp();

  void bind(const int _port);
  void listen();

protected:
  int socket_;
  struct sockaddr_in hint_;
  int port_;
  const Router *router_;

  virtual Client *await_client();
  void connect(Client *_client);
};

static bool SSLLIBINIT = false;

class Tls : public Tcp
{
public:
  Tls(const Router *_router, const std::string _cert_path, const std::string _key_path);
  ~Tls();

private:
  SSL_CTX *ctx_;

  Client *await_client() override;
};
