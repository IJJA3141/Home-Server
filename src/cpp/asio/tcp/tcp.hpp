#pragma once

#include "../asio.hpp"
#include "../executor.hpp"
#include "../reactor.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <netinet/in.h>
#include <sys/socket.h>

// clang-format off
#if EAGAIN == EWOULDBLOCK
#define EAGAIN_EWOULDBLOCK EAGAIN
#else
#define EAGAIN_EWOULDBLOCK EAGAIN: case EWOULDBLOCK
#endif
// clang-format on

namespace asio
{

namespace tcp
{

class Acceptor;

class Socket : public SocketBase
{
public:
  static Awaitable<std::expected<Socket, Error>> async_connect(Executor* executor, Reactor* reactor,
                                                               std::span<const char, INET_ADDRSTRLEN> remote_addr,
                                                               std::uint16_t remote_port);

  // SocketBase manages expected, reactor deregistration and fd closeing
  ~Socket() noexcept = default;

  Socket(Socket&&);
  Socket& operator=(Socket&&);

protected:
  Socket(Executor* executor, Reactor* reactor, int fd, sockaddr_in addr);
  friend Acceptor;

private:
  sockaddr_in addr_;

  Executor::Operation read(std::span<std::byte> buffer, std::pair<std::size_t, Error>& value) override final;
  Executor::Operation write(std::span<const std::byte> buffer,
                            std::pair<std::size_t, Error>& value) override final;
};
static_assert(std::movable<Socket>);
static_assert(!std::copyable<Socket>);

class Acceptor
{
public:
  Acceptor(Executor* executor, Reactor* reactor, std::uint32_t addr, std::uint16_t port, std::size_t n);
  ~Acceptor() noexcept;

  Acceptor(const Acceptor&) = delete;
  Acceptor& operator=(const Acceptor&) = delete;

  Acceptor(Acceptor&&);
  Acceptor& operator=(Acceptor&&);

  Awaitable<std::expected<Socket, Error>> async_accept() const;

protected:
  Executor* executor_;
  Reactor* reactor_;

  int fd_;
  sockaddr_in addr_;

  void close() noexcept;
  Executor::Operation accept(std::expected<Socket, Error>& value) const;
};
static_assert(std::movable<Acceptor>);
static_assert(!std::copyable<Acceptor>);

} // namespace tcp
} // namespace asio
