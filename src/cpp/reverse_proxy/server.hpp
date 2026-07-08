#pragma once

#include "../common/ring_buffer.hpp"
#include "../common/uuid.hpp"
#include "../config.hpp"
#include "forward_table.hpp"

#include "epoll.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <sys/epoll.h>
#include <unordered_map>

class TcpServer
{
protected:
  struct Client;

public:
  TcpServer(const std::string& ip, uint16_t port, Epoll epoll, ForwardingTable forwarding_table);
  ~TcpServer();

  void listen(); // === start
  inline int fd() { return this->listening_socket_; }
  Client* find(Uuid client_uuid); // can be nullptr to indicated not found

  // accept
  void virtual notify_read();
  void notify_half_close();
  void notify_close();
  void notify_error();

protected:
  int listening_socket_;

  Epoll epoll_;
  ForwardingTable forwarding_table_;
  std::unordered_map<Uuid, Client*> client_pool;

  void remove_child(Uuid child_uuid);
};

struct TcpServer::Client
{
  using RingBuffer = RingBuffer<std::byte, REVERSE_PROXY_CLIENT_RING_BUFFER_SIZE>;
  using Buffer = std::array<std::byte, REVERSE_PROXY_CLIENT_BUFFER_SIZE>;

  Client(int listening_socket_, const ForwardingTable& forwarding_table);
  virtual ~Client();

  virtual void recv();
  virtual void send(std::span<std::byte>);

  void notify_read();
  void notify_write();
  void notify_half_close();
  void notify_close();
  void notify_error();

  inline int fd() { return socket; }
  inline Uuid uuid() { return uuid_; }

protected:
  RingBuffer out_buffer_; // buffers responses that could not be send in one go
  Buffer in_buffer_;      // used to read incoming request
  int socket;
  Uuid uuid_;
  sockaddr_in addr_; // stores client infos
  std::string ip_;
  ForwardingTable forwarding_table_;
};

class TlsServer : public TcpServer
{
public:
  static std::atomic<int> SSL_LIB_INIT;

  TlsServer(const std::string& ip, uint16_t port, Epoll epoll, ForwardingTable forwarding_table,
            const std::filesystem::path& certificat, const std::filesystem::path& key);
  ~TlsServer();

  // accept
  void notify_read() override final;

protected:
  struct Client;
  SSL_CTX* ctx_;
};

struct TlsServer::Client final : TcpServer::Client
{
  Client(int listening_socket, TlsServer& parent);
  ~Client() override final;

  void recv() override final;
  void send(std::span<std::byte>) override final;

private:
  ::SSL* ssl_;
  bool handshake_established;
};
