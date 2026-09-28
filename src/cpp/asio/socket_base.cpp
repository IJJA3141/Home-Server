#include "../common/assert.hpp"
#include "asio.hpp"

#include <algorithm>
#include <cstddef>
#include <unistd.h>

using namespace asio;

SocketBase::SocketBase(Executor* executor, Reactor* reactor, int fd)
    : executor_{executor}, reactor_{reactor}, fd_{fd}
{
  assert(executor != nullptr);
  assert(reactor != nullptr);
  assert(fd >= 0);

  assert(!executor->has(fd), "fd should not be registered");
  assert(!reactor->has(fd), "fd should not be registered");

  this->executor_->add(this->fd_);
  this->reactor_->add(this->fd_);

  return;
}

SocketBase::~SocketBase() noexcept
{
  if (this->executor_ == nullptr && this->reactor_ == nullptr && this->fd_ < 0) return;
  this->close();
  return;
}

SocketBase::SocketBase(SocketBase&& other) noexcept
    : executor_{other.executor_}, reactor_{other.reactor_}, fd_{other.fd_}
{
  assert(other.executor_ != nullptr);
  assert(other.reactor_ != nullptr);
  assert(other.fd_ >= 0);

  assert(other.executor_->has(other.fd_));
  assert(other.reactor_->has(other.fd_));

  other.executor_ = nullptr;
  other.reactor_ = nullptr;
  other.fd_ = -1;

  return;
};

SocketBase& SocketBase::operator=(SocketBase&& other) noexcept
{
  assert(this != &other, "socket = std::move(socket); really?");

  assert(other.executor_ != nullptr);
  assert(other.reactor_ != nullptr);
  assert(other.fd_ >= 0);

  assert(other.executor_->has(other.fd_));
  assert(other.reactor_->has(other.fd_));

  // if one is true but another is false close will assert
  if (this->executor_ != nullptr || this->reactor_ != nullptr || this->fd_ >= 0) this->close();

  // move
  this->executor_ = std::move(other.executor_);
  this->reactor_ = std::move(other.reactor_);
  this->fd_ = std::move(other.fd_);

  // reset
  other.executor_ = nullptr;
  other.reactor_ = nullptr;
  other.fd_ = -1;

  return *this;
};

void SocketBase::close() noexcept
{
  assert(this->executor_ != nullptr);
  assert(this->reactor_ != nullptr);
  assert(this->fd_ >= 0);

  this->executor_->del(this->fd_);
  this->reactor_->del(this->fd_);
  assert(::close(this->fd_));

  this->executor_ = nullptr;
  this->reactor_ = nullptr;
  this->fd_ = -1;

  return;
}

///
// methods
///

using AwtBtErr = Awaitable<std::pair<std::size_t, Error>>;

AwtBtErr SocketBase::async_read(std::span<std::byte> buffer)
{ // clang-format off
  // assert valid socket state for read
  assert(buffer.data() != nullptr);
  assert(this->executor_ != nullptr);
  assert(this->reactor_ != nullptr);
  assert(this->executor_->has(this->fd_));
  assert(this->reactor_->has(this->fd_));
  assert(this->fd_ >= 0);

  return {this->fd_, this->executor_, Executor::Operation::read, {0, Error::none},
    [this, buffer](auto& value)
    { 
      // retrieve what as been saved in value from coro
      std::size_t read_bytes = value.first;
      std::span<std::byte> free_buffer = buffer.subspan(read_bytes);

      // call
      Executor::Operation op = this->read(free_buffer, value);
      value.first = read_bytes + value.first;

      // underlinng call needs an operation, forward it to the executor
      if (op == Executor::Operation::end) return op;

      // need another call
      if (value.first < buffer.size() && value.second == Error::none) return Executor::Operation::pri_read;

      return Executor::Operation::end;
    }
  };
} // clang-format on

AwtBtErr SocketBase::async_read_once(std::span<std::byte> buffer)
{ // clang-format off
  // assert valid socket state for read
  assert(buffer.data() != nullptr);
  assert(this->executor_ != nullptr);
  assert(this->reactor_ != nullptr);
  assert(this->executor_->has(this->fd_));
  assert(this->reactor_->has(this->fd_));
  assert(this->fd_ >= 0);
  
  return {this->fd_, this->executor_, Executor::Operation::read, {0, Error::none},
    [this, buffer](auto& value) { return this->read(buffer, value); }
  };
} // clang-format on

AwtBtErr SocketBase::async_read(std::span<std::byte> buffer, std::span<const std::byte> delimiter)
{ // clang-format off
  // assert valid socket state for read
  assert(buffer.data() != nullptr);
  assert(delimiter.data() != nullptr);
  assert(this->executor_ != nullptr);
  assert(this->reactor_ != nullptr);
  assert(this->executor_->has(this->fd_));
  assert(this->reactor_->has(this->fd_));
  assert(this->fd_ >= 0);
  
  return {this->fd_, this->executor_, Executor::Operation::read, {0, Error::none},
    [this, buffer, delimiter](auto& value)
    {
      // retrieve what as been saved in value from coro
      std::size_t read_bytes = value.first;
      std::span<std::byte> free_buffer = buffer.subspan(read_bytes);

      // call
      Executor::Operation op = this->read(free_buffer, value);
      value.first = read_bytes + value.first;

      // underlinng call needs an operation, forward it to the executor
      if (op == Executor::Operation::end) return op;

      // an erorr occured or condition is true
      if (value.second != Error::none || std::ranges::contains_subrange(buffer, delimiter))
        return Executor::Operation::end;

      if (value.first >= buffer.size()) 
      {
        value.second = Error::buffer_full;
        return Executor::Operation::end;
      }

      // retury
      return Executor::Operation::pri_read;
    }
  };
} // clang-format on

AwtBtErr SocketBase::async_write(std::span<const std::byte> buffer)
{ // clang-format off
  // assert valid socket state for read
  assert(buffer.data() != nullptr);
  assert(this->executor_ != nullptr);
  assert(this->reactor_ != nullptr);
  assert(this->executor_->has(this->fd_));
  assert(this->reactor_->has(this->fd_));
  assert(this->fd_ >= 0);

  return {this->fd_, this->executor_, Executor::Operation::write, {0, Error::none},
    [this, buffer](auto& value)
    {
      // retrieve what as been saved in value from coro
      std::size_t read_bytes = value.first;
      std::span<const std::byte> unused_buffer = buffer.subspan(read_bytes);

      // call
      Executor::Operation op = this->write(unused_buffer, value);
      value.first = read_bytes + value.first;

      // underlinng call needs an operation, forward it to the executor
      if (op == Executor::Operation::end) return op;

      // need another call
      if (value.first < buffer.size() && value.second == Error::none) return Executor::Operation::pri_write;

      return Executor::Operation::end;
    }
  };
} // clang-format on

AwtBtErr SocketBase::async_write_once(std::span<const std::byte> buffer)
{ // clang-format off
  assert(buffer.data() != nullptr);
  assert(this->executor_ != nullptr);
  assert(this->reactor_ != nullptr);
  assert(this->executor_->has(this->fd_));
  assert(this->reactor_->has(this->fd_));
  assert(this->fd_ >= 0);

  return {this->fd_, this->executor_, Executor::Operation::write, {0, Error::none},
    [this, buffer](auto& value) { return this->write(buffer, value); }
  };
} // clang-format on
