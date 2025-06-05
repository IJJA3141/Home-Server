#pragma once

#include "client.hpp"
#include "router.hpp"

#include <thread>

class Tcp {
public:
  static const size_t CLIENT_SIZE = 10;
  std::thread thread;

  Tcp(const Router *_router);
  ~Tcp();

  void bind(const int _port);
  void listen();

protected:
  const Router *router_;
  const Client *client_array_[CLIENT_SIZE]; // has ownership over the clients

  // socket stuff
  int socket_;
  struct sockaddr_in hint_;
  int port_;

  virtual Client *await_client();
  void connect(const size_t _index); // should delete clients
  int new_client_index();
};

static bool SSLLIBINIT = false;
class Tls : public Tcp {
public:
  Tls(const size_t _client_size, const Router *_router, const std::string _cert_path, const std::string _key_path);
  ~Tls();

private:
  SSL_CTX *ctx_;

  Client *await_client() override;
};
