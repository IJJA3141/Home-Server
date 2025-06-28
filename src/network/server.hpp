#pragma once

#include "client.hpp"
#include "router.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <openssl/crypto.h>
#include <sys/epoll.h>
#include <thread>

class Tcp
{
public:
  Router router;

  Tcp(const size_t _pool_size);
  ~Tcp();

  bool is_running() const;

  void bind(const int _port);
  void listen();
  void close();

protected:
  std::thread client_bay_;
  std::thread client_pool_;

  std::map<int, std::unique_ptr<Client>> clients_;
  bool running_;

  // socket stuff
  int socket_;
  sockaddr_in hint_;

  // epoll
  int epoll_;
  const size_t pool_size_;
  epoll_event * const events_;

  virtual std::unique_ptr<Client> await_client();
};

class Tls : public Tcp
{
public:
  Tls(const size_t _pool_size, const std::filesystem::path& _cert, const std::filesystem::path& _key);
  ~Tls();

private:
  static bool SSLLIBINIT;

  SSL_CTX* ctx_;

  std::unique_ptr<Client> await_client() override;
};
