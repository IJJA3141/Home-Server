#pragma once

#include "../common/ring_buffer.hpp"
#include "../common/uuid.hpp"
#include "../config.hpp"
#include "../ipc/ipc.hpp"
#include "../protocol/forwarding/forwarding.hpp"
#include "epoll.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <netinet/in.h>
#include <openssl/crypto.h>
#include <string>
#include <sys/epoll.h>
#include <unordered_map>
#include <unordered_set>

class TcpServer
{
protected:
  struct Client;
  using Table = std::unordered_map<std::string, ipc::IClient*>;
  using Pool = std::unordered_map<Uuid, Client*>;

public:
  TcpServer(const std::string& ip, uint16_t port, Epoll epoll, const Table& forwarding);
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
  const Table& forwarding_;
  Epoll epoll_;
  Pool clients_;

  void drop_child(Uuid child_uuid);
};

struct TcpServer::Client
{
  using RingBuffer = RingBuffer<std::byte, REVERSE_PROXY_CLIENT_RING_BUFFER_SIZE>;
  using Buffer = std::array<std::byte, REVERSE_PROXY_CLIENT_BUFFER_SIZE>;
  using ParserContext = protocol::forwarding::Request::ParserContext;

  Client(int listening_socket, TcpServer& parent);
  virtual ~Client();

  virtual bool recv();
  virtual void send(std::span<const std::byte>);

  void notify_read();
  void notify_write();
  void notify_half_close();
  void notify_close();
  void notify_error();

  inline int fd() { return socket_; }
  inline Uuid uuid() { return uuid_; }

protected:
  RingBuffer out_; // buffers responses that could not be send in one go
  RingBuffer in_;  // used to read incoming request
  int socket_;

  Uuid uuid_;
  sockaddr_in addr_; // stores client infos
  std::string ip_;   // same but readable

  TcpServer& parent;
  std::unordered_set<ipc::IClient*> in_use_;

  ParserContext ctx_;
  ipc::IClient* host_;

  size_t max_header_size;
  size_t remaining;

  bool get_host(std::span<const char> chars);
};

class TlsServer : public TcpServer
{
public:
  static std::atomic<int> SSL_LIB_INIT;

  TlsServer(const std::string& ip, uint16_t port, Epoll epoll, const Table& forwarding,
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

  bool recv() override final;
  void send(std::span<const std::byte>) override final;

private:
  ::SSL* ssl_;
  bool handshake_established;
};
