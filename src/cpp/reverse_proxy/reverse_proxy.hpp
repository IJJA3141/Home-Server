#pragma once

#include "../common/ring_buffer.hpp"
#include "../config.hpp"
#include "../protocol/protocol.hpp"
#include "policy.hpp"
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <string>
#include <sys/epoll.h>

struct A
{
  enum class State
  {
    Method,
    Path,
    Version,
    Header,
    Forwarding

  } state;
};

class TcpServer
{
public:
  TcpServer(const std::string& ip, uint16_t port, protocol::Handler<ForwardPolicy> handler);
  ~TcpServer();

  void listen();

protected:
  struct Client
  {
    char ip[INET_ADDRSTRLEN];
    uint16_t port;
    int socket;
    int epoll;
    bool connection_failed;
    const protocol::Handler<ForwardPolicy>& handler;
    RingBuffer<std::byte, REVERSE_PROXY_CLIENT_BUFFER_SIZE> connection_buffer;

    A a;

    Client(int listening_socket, int epoll, const protocol::Handler<ForwardPolicy>& handler);
    virtual ~Client();

    void client_event(uint32_t events);
    virtual ssize_t recv(int __fd, void* __buf, size_t __nbytes);
    virtual ssize_t send(int fd, const void* buf, size_t n, int flags);
  };

  static constexpr auto connection_type_ = CONNECTION_TYPE_INSECURE;
  int listening_socket_;
  int epoll_fd_;
  bool running_;
  protocol::Handler<ForwardPolicy> handler_;

  virtual void accept();
  void server_event(uint32_t events);
};

class TlsServer : public TcpServer
{
public:
  static std::atomic<int> SSL_LIB_INIT;

  TlsServer(const std::string& ip, uint16_t port, const std::filesystem::path& certificat,
            const std::filesystem::path& key, protocol::Handler<ForwardPolicy> handler);
  ~TlsServer();

private:
  static constexpr auto connection_type_ = CONNECTION_TYPE_SECURE;
  SSL_CTX* ctx_;

  void accept() override final;

  struct Client final : TcpServer::Client
  {
    ::SSL* ssl_;
    bool handshake_established;

    Client(int listening_socket, int epoll, SSL_CTX* ctx, const protocol::Handler<ForwardPolicy>& handler);
    ~Client() override final;

    ssize_t recv(int __fd, void* __buf, size_t __nbytes) override final;
    ssize_t send(int fd, const void* buf, size_t n, int flags) override final;
    void handle_error(int err);
    void handshake();
  };
};
