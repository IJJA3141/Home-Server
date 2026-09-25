#include "asio.hpp"

#include <algorithm>

using namespace asio;
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
