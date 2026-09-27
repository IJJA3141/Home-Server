#pragma once

#include "executor.hpp"
#include "reactor.hpp"

#include <cstring>
#include <unistd.h>

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

// base class for aio socket stream
class SocketBase
{
public:
  SocketBase(Executor* executor, Reactor* reactor, int fd);
  ~SocketBase() noexcept;

  // noncopyable
  SocketBase(const SocketBase&) = delete;
  SocketBase& operator=(const SocketBase&) = delete;

  // movable
  SocketBase(SocketBase&&) noexcept;
  SocketBase& operator=(SocketBase&&) noexcept;

  // standard return type of async io operation (size of bytes received in buffer, and state of action)
  using io_rt = Awaitable<std::pair<std::size_t, Error>>;

  io_rt async_read(std::span<std::byte> buffer);
  io_rt async_read_once(std::span<std::byte> buffer);
  io_rt async_read(std::span<std::byte> buffer, std::span<const std::byte> delimiter);

  io_rt async_write(std::span<const std::byte> buffer);
  io_rt async_write_once(std::span<const std::byte> buffer);

protected:
  Executor* executor_;
  Reactor* reactor_;
  int fd_;

  virtual Executor::Operation read(std::span<std::byte> buffer, std::pair<std::size_t, Error>& value) = 0;
  virtual Executor::Operation write(std::span<const std::byte> buffer, std::pair<std::size_t, Error>& value) = 0;

private:
  void close() noexcept;
};

} // namespace asio
