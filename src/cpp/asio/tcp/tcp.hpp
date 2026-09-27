#pragma once

#include "../asio.hpp"
#include "../executor.hpp"
#include "../reactor.hpp"

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
  static Awaitable<std::expected<Socket, Error>> async_connect(Executor*, Reactor*,
                                                               std::span<const char, INET_ADDRSTRLEN> addr,
                                                               std::uint16_t port);

  void close();

protected:
  Socket(Executor* executor, Reactor* reactor, int fd, sockaddr_in addr);
  friend Acceptor;

private:
  sockaddr_in addr_;

  Executor::Operation read(std::span<std::byte> buffer, std::pair<std::size_t, Error>& value) override final;
  Executor::Operation write(std::span<const std::byte> buffer,
                            std::pair<std::size_t, Error>& value) override final;
};

class Acceptor
{
public:
  Acceptor(Executor* executor, Reactor* reactor, std::uint32_t addr, std::uint16_t port, std::size_t n);

  Awaitable<std::expected<Socket, Error>> async_accept();

  void close();

  Acceptor(const Acceptor&) = delete;
  Acceptor& operator=(const Acceptor&) = delete;

  Acceptor(const Acceptor&& other)
      : executor_{other.executor_}, reactor_{other.reactor_}, fd_{other.fd_}, addr_{other.addr_}
  {
    if (other.executor_ == nullptr) throw std::invalid_argument("nullptr executor");
    if (other.reactor_ == nullptr) throw std::invalid_argument("nullptr reactor");
    if (other.fd_ < 0) throw std::invalid_argument("fd < 0");

    if (!this->executor_->has(this->fd_)) throw std::invalid_argument("unregistered executor");
    if (!this->reactor_->has(this->fd_)) throw std::invalid_argument("unregistered reactor");

    return;
  };
  Acceptor& operator=(const Acceptor&&);

protected:
  Executor* executor_;
  Reactor* reactor_;

  int fd_;
  sockaddr_in addr_;

  Executor::Operation accept(std::expected<Socket, Error>& value);
};
static_assert(std::movable<Acceptor>);
static_assert(!std::copyable<Acceptor>);

} // namespace tcp
} // namespace asio
