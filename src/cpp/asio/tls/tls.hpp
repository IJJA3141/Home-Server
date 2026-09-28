#pragma once

#include "../asio.hpp"
#include "../executor.hpp"
#include "../tcp/tcp.hpp"

#include <expected>
#include <filesystem>
#include <openssl/crypto.h>

namespace asio
{

namespace tls
{

enum class SSL_State
{
  handshake,
  established,
  shutdown
};

class Socket : public SocketBase
{
public:
  static Awaitable<std::expected<Socket, Error>> async_connect(tcp::Socket&& socket, SSL* ssl);

  ~Socket() noexcept;

  // move op
  Socket(Socket&& socket);
  Socket& operator=(Socket&&);

protected:
  Socket(tcp::Socket&& socket, SSL* ssl);

private:
  SSL_State state_ = SSL_State::handshake;
  SSL* ssl_;

  Executor::Operation read(std::span<std::byte> buffer, std::pair<std::size_t, Error>& value) override final;
  Executor::Operation write(std::span<const std::byte> buffer,
                            std::pair<std::size_t, Error>& value) override final;
};
static_assert(std::movable<Socket>);
static_assert(!std::copyable<Socket>);

class Acceptor : private tcp::Acceptor
{
public:
  Acceptor(const std::filesystem::path& certificat, const std::filesystem::path& key, Executor* executor,
           Reactor* reactor, std::uint32_t addr, std::uint16_t port, std::size_t n);

  Acceptor(Acceptor&&);
  Acceptor& operator=(Acceptor&&);

  Awaitable<std::expected<Socket, Error>> async_accept() const;

private:
  SSL_CTX* ctx_;
};
static_assert(std::movable<Acceptor>);
static_assert(!std::copyable<Acceptor>);

} // namespace tls
} // namespace asio
