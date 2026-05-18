#pragma once

#include "../config.hpp"
#include "../protocol/http/http.hpp"
#include "../utils/buffer.hpp"
#include <arpa/inet.h>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/crypto.h>
#include <string>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using Handler = protocol::Handler<protocol::HTTP>;

class TcpServer
{
public:
  TcpServer(const std::string& ip, uint16_t port, const Handler request_handler);
  void listen();

protected:
  const std::string ip_;
  const uint16_t port_;
  int listening_socket_;
  int epoll_fd_;
  bool running_;
  const Handler request_handler_;

  virtual void accept();

  struct Client
  {
    char ip[INET_ADDRSTRLEN];
    uint16_t port;
    int socket;
    int epoll;
    const Handler request_handler;

    ipc::RingBuffer<char, REVERSE_PROXY_CLIENT_BUFFER_SIZE> connection_buffer;
    protocol::HTTP::Request::ParserContext parser_ctx;

    Client(const int listening_socket, const int epoll, const Handler request_handler);
    ~Client();

    bool notify();
    virtual inline const std::string connection_type() const { return "http"; }
    virtual ssize_t recv(int __fd, void* __buf, size_t __n, int __flags);
    virtual ssize_t send(int __fd, const void* __buf, size_t __n, int __flags);
  };
};

class TlsServer : public TcpServer
{
public:
  static std::atomic<size_t> SSL_LIB_HANDLE;
  static std::atomic<bool> SSL_LIB_INIT;

  TlsServer(const std::string& ip, uint16_t port, const std::filesystem::path& certificat,
            const std::filesystem::path& key, const Handler request_handler);
  ~TlsServer();

private:
  SSL_CTX* ctx_;

  void accept() override;

  struct Client : TcpServer::Client
  {
    Client(const int listening_socket, const int epoll, SSL_CTX* const ctx, const Handler request_handler);
    ~Client();

    inline const std::string connection_type() const override { return "https"; }
    ssize_t recv(int __fd, void* __buf, size_t __n, int __flags) override;
    ssize_t send(int __fd, const void* __buf, size_t __n, int __flags) override;

  private:
    ::SSL* ssl_;
  };
};
