#include "../common/logger.hpp"
#include "asio.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <stdexcept>

using namespace asio;

SocketBase::SocketBase(Executor* executor, Reactor* reactor, int fd)
    : executor_{executor}, reactor_{reactor}, fd_{fd}
{
  if (executor == nullptr) throw std::invalid_argument("nullptr executor");
  if (reactor == nullptr) throw std::invalid_argument("nullptr reactor");
  if (fd < 0) throw std::invalid_argument("fd < 0");

  this->executor_->add(this->fd_);
  this->reactor_->add(this->fd_);

  return;
}

SocketBase::SocketBase(SocketBase&& other) noexcept
    : executor_{std::move(other.executor_)}, reactor_{std::move(other.reactor_)}, fd_{std::move(other.fd_)}
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
  assert(this != &other);

  assert(other.executor_ != nullptr);
  assert(other.reactor_ != nullptr);
  assert(other.fd_ >= 0);

  assert(other.executor_->has(other.fd_));
  assert(other.reactor_->has(other.fd_));

  this->close();

  this->executor_ = std::move(other.executor_);
  this->reactor_ = std::move(other.reactor_);
  this->fd_ = std::move(other.fd_);

  other.executor_ = nullptr;
  other.reactor_ = nullptr;
  other.fd_ = -1;

  return *this;
};

void SocketBase::close() noexcept
{
  assert(this->executor_ != nullptr);
  assert(this->reactor_ != nullptr);
  assert(this->fd_ > 0);

  this->executor_->del(this->fd_);
  this->reactor_->del(this->fd_);
  ::close(this->fd_);

  return;
}

SocketBase::~SocketBase() noexcept
{
  if (this->fd_ < 0 && this->executor_ == nullptr && this->reactor_ == nullptr)
  { // this is null should only be possible if moved
    return;
  }
  else if (this->fd_ < 0 || this->executor_ == nullptr || this->reactor_ == nullptr)
  { // this is an exception and should never occur
    bool hasEx = this->executor_ != nullptr;
    bool hasRk = this->reactor_ != nullptr;
    bool hasFd = this->fd_ < 0;
    auto log = Logger::get("SocketBase");
    log.error("destructor of zombie (executor={}, reactor={}, fd={})", hasEx, hasRk, hasFd);
    return;
  }

  this->executor_->del(this->fd_);
  this->reactor_->del(this->fd_);
  if (::close(this->fd_))
  {
    auto log = Logger::get("SocketBase", [] { return std::strerror(errno); });
    log.error("destructor could not close fd");
  }

  return;
}

// methods

using AwtBtErr = Awaitable<std::pair<std::size_t, Error>>;

AwtBtErr SocketBase::async_read(std::span<std::byte> buffer)
{ // clang-format off
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
  return {this->fd_, this->executor_, Executor::Operation::read, {0, Error::none},
    [this, buffer](auto& value) { return this->read(buffer, value); }
  };
} // clang-format on

AwtBtErr SocketBase::async_read(std::span<std::byte> buffer, std::span<const std::byte> delimiter)
{ // clang-format off
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
  return {this->fd_, this->executor_, Executor::Operation::write, {0, Error::none},
    [this, buffer](auto& value) { return this->write(buffer, value); }
  };
} // clang-format on
