#pragma once

#include "client.hpp"
#include "router.hpp"

#include <filesystem>
#include <set>

class Tcp
{
public:
  Tcp(int _port, const Router& _router);
  ~Tcp();

  inline bool is_running() const { return running_; }

  void listen();
  void close();

protected:
  sockaddr_in addr_;
  int socket_;
  int epoll_;

private:
  std::set<Client*> client_bay_;
  const Router router_;
  bool running_;

  virtual Client* anchor_client() const;
};

class Tls : public Tcp
{
public:
  Tls(int _port, const std::filesystem::path& _cert, const std::filesystem::path& _key, const Router& _router);
  ~Tls();

private:
  static bool SSLLIBINIT;
  SSL_CTX* ctx_;

  Client* anchor_client() const override;
};

// epoll_event socket_event_;
// epoll_event client_event_[10];
