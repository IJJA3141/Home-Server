#pragma once

#include "executor.hpp"
#include "reactor.hpp"

namespace asio
{

enum class Error
{
  none = 0,
  eof,

  invalid_argumnent,
  socket_event,
  system_error,
  buffer_full,
  closed,
  unknown
};

template <typename R> struct Awaitable
{
  int fd;
  Executor* executor;
  Executor::Operation op;
  R value;
  std::function<Executor::Operation(R&)> executable;

  constexpr bool await_ready() noexcept { return false; };
  void await_suspend(std::coroutine_handle<> handle)
  {
    executor->execute(op, fd, std::bind(executable, std::ref(value)), handle);
  }
  R&& await_resume() noexcept { return std::move(value); };
};

class SocketBase
{
public:
  Awaitable<std::pair<std::size_t, Error>> async_read(std::span<std::byte> buffer);
  Awaitable<std::pair<std::size_t, Error>> async_read_once(std::span<std::byte> buffer);
  Awaitable<std::pair<std::size_t, Error>> async_read(std::span<std::byte> buffer,
                                                      std::span<const std::byte> delimiter);

  Awaitable<std::pair<std::size_t, Error>> async_write(std::span<const std::byte> buffer);
  Awaitable<std::pair<std::size_t, Error>> async_write_once(std::span<const std::byte> buffer);

protected:
  SocketBase(const SocketBase&& other) : executor_{other.executor_}, reactor_{other.reactor_}, fd_{other.fd_}
  {
    if (other.executor_ == nullptr) throw std::invalid_argument("nullptr executor");
    if (other.reactor_ == nullptr) throw std::invalid_argument("nullptr reactor");
    if (other.fd_ < 0) throw std::invalid_argument("fd < 0");

    if(!this->executor_->has(this->fd_)) throw std::invalid_argument("unregistered executor");
    if(!this->reactor_->has(this->fd_)) throw std::invalid_argument("unregistered reactor");

    return;
  };

  SocketBase(Executor* executor, Reactor* reactor, int fd) : executor_{executor}, reactor_{reactor}, fd_{fd}
  {
    if (executor == nullptr) throw std::invalid_argument("nullptr executor");
    if (reactor == nullptr) throw std::invalid_argument("nullptr reactor");
    if (fd < 0) throw std::invalid_argument("fd < 0");

    this->executor_->add(this->fd_);
    this->reactor_->add(this->fd_);

    return;
  }

  Executor* executor_;
  Reactor* reactor_;
  int fd_;

  virtual Executor::Operation read(std::span<std::byte> buffer, std::pair<std::size_t, Error>& value) = 0;
  virtual Executor::Operation write(std::span<const std::byte> buffer, std::pair<std::size_t, Error>& value) = 0;
};

} // namespace asio
