#pragma once

#include "client.hpp"
#include "router.hpp"

#include <netinet/in.h>
#include <openssl/crypto.h>
#include <string>
#include <thread>

class Tcp
{
public:
  struct {
    bool listening : 1;
    bool stop : 1;
  } state;
  std::thread thread;

  Tcp(const size_t _client_size, const Router *_router);
  ~Tcp();

  void bind(const int _port);
  void listen();

  void update_client_state();
  int clean_client_array();         // return index of last removed client (-1 if none)
  std::string client_array_state(); // mainly for debugging / login

protected:
  const Router *router_;
  const size_t client_size_;
  Client **client_array_; // has ownership over the clients 

  // socket stuff
  int socket_;
  struct sockaddr_in hint_;
  int port_;

  virtual Client *await_client();
  void connect(const size_t _index); // should delete clients
  int inactive_client_index();       // -1 if none
};

static bool SSLLIBINIT = false;
class Tls : public Tcp
{
public:
  Tls(const size_t _client_size, const Router *_router, const std::string _cert_path,
      const std::string _key_path);
  ~Tls();

private:
  SSL_CTX *ctx_;

  Client *await_client() override;
};
