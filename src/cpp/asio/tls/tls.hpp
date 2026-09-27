#pragma once

#include "../asio.hpp"
#include "../executor.hpp"
#include "../tcp/tcp.hpp"

#include <concepts>
#include <expected>
#include <openssl/crypto.h>

class noncopyable
{
public:
  noncopyable(const noncopyable&) = delete;
  const noncopyable& operator=(const noncopyable&) = delete;

protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

namespace asio
{

namespace tls
{

class Socket : public SocketBase, public noncopyable
{
  struct Context
  {
    SSL* ssl;
    enum
    {
      handshake,
      established,
      shutdown
    } state;
  };

public:
  // convertion from tcp to tls
  Socket(const tcp::Socket&& socket, SSL_CTX* const _ctx);

  // move op
  Socket(const Socket&& socket);
  Socket& operator=(const Socket&&);

private:
  Context ctx_;

  Executor::Operation read(std::span<std::byte> buffer, std::pair<std::size_t, Error>& value) override final;
  Executor::Operation write(std::span<const std::byte> buffer,
                            std::pair<std::size_t, Error>& value) override final;
};
static_assert(std::movable<Socket>);
static_assert(!std::copyable<Socket>);

//
//
//
//
//
//
//

class Acceptor : tcp::Acceptor
{
public:
  Awaitable<std::expected<Socket, Error>> async_accept();

private:
  SSL_CTX* ctx_;
};
static_assert(std::movable<Acceptor>);
static_assert(!std::copyable<Acceptor>);

} // namespace tls
} // namespace asio
