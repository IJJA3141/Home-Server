#include "../../common/assert.hpp"
#include "tcp.hpp"

#include <arpa/inet.h>
#include <expected>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace asio;
using namespace tcp;

// TODO error handling

Awaitable<std::expected<Socket, Error>> Socket::async_connect(Executor* executor, Reactor* reactor,
                                                              std::span<const char, INET_ADDRSTRLEN> peer_addr,
                                                              std::uint16_t peer_port)
{
  assert(executor != nullptr);
  assert(reactor != nullptr);
  assert(peer_addr.data() != nullptr);

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(peer_port);

  if (inet_aton(peer_addr.data(), &addr.sin_addr) == 0)
  {
    // handle err
    throw;
  }

  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (fd < 0)
  {
    // handle error
    throw;
  }

  // clang-format off
  return {fd, executor, Executor::Operation::read, Socket(executor, reactor, fd, addr), 
    [](auto& value) 
    { 
      assert(value);
      Reactor::Events events = (*value->reactor_)[value->fd_];

      // conditions
      if (events.error)
      {
        value = std::unexpected{Error::system_error};
        return Executor::Operation::end;
      }
      else if (events.socket)
      {
        value = std::unexpected{Error::socket_event};
        return Executor::Operation::end;
      }
      else if (!events.read)
      {
        return Executor::Operation::pri_read;
      }

      socklen_t len = sizeof value->addr_;
      const sockaddr* addr = reinterpret_cast<const sockaddr*>(&value->addr_);

      // action & results
      if (::connect(value->fd_, addr, len)) switch (errno)
      {
      case EINPROGRESS:
        value->reactor_->reset_read(value->fd_);
        return Executor::Operation::pri_read;

      case EADDRNOTAVAIL:
      case EAFNOSUPPORT:
        value = std::unexpected{Error::invalid_argument};
        break;

      case ECONNREFUSED:
        value = std::unexpected{Error::system_error};
        break;

      defautl:
        value = std::unexpected{Error::unknown};
        break;
      }

      return Executor::Operation::end;
    } // clang-format on
  };
}

Socket::Socket(Executor* executor, Reactor* reactor, int fd, sockaddr_in addr)
    : SocketBase(executor, reactor, fd), addr_{addr}
{
}

Socket::Socket(Socket&& other) : SocketBase(std::move(other)), addr_{std::move(other.addr_)} {};
Socket& Socket::operator=(Socket&& other)
{
  SocketBase::operator=(std::move(other));
  this->addr_ = std::move(other.addr_);
  return *this;
};

Executor::Operation Socket::read(std::span<std::byte> buffer, std::pair<std::size_t, Error>& value)
{
  Reactor::Events events = (*this->reactor_)[this->fd_];

  // conditions
  if (events.error)
  {
    value.second = Error::system_error;
    return Executor::Operation::end;
  }
  else if (events.socket)
  {
    value.second = Error::socket_event;
    return Executor::Operation::end;
  }
  else if (!events.read)
  {
    value.second = Error::none;
    return Executor::Operation::pri_read;
  }

  // action
  ssize_t bytes = recv(this->fd_, buffer.data(), buffer.size(), MSG_DONTWAIT);

  // results
  if (bytes > 0)
  {
    value = {bytes, Error::none};
    return Executor::Operation::end;
  }
  else if (bytes == 0 || events.closed)
  {
    value.second = Error::eof;
    return Executor::Operation::end;
  }
  else switch (errno) // clang-format off
  { 
  case EAGAIN_EWOULDBLOCK:
    this->reactor_->reset_read(this->fd_);
    return Executor::Operation::pri_read;

  case EBADF:
  case EFAULT:
  case EINVAL:
  case EISDIR:
    value.second = Error::invalid_argument;
    break;

  case EINTR:
  case EIO:
    value.second = Error::system_error;
    break;

  defautl:
    value.second = Error::unknown;
    break;

  } // clang-format on

  return Executor::Operation::end;
}

Executor::Operation Socket::write(std::span<const std::byte> buffer, std::pair<std::size_t, Error>& value)
{
  Reactor::Events events = (*this->reactor_)[this->fd_];

  // conditions
  if (events.socket)
  {
    value = {0, Error::socket_event};
    return Executor::Operation::end;
  }

  if (events.closed)
  {
    value = {0, Error::closed};
    return Executor::Operation::end;
  }

  if (!events.write)
  {
    value = {0, Error::none};
    return Executor::Operation::pri_write;
  }

  // action
  ssize_t bytes = send(this->fd_, buffer.data(), buffer.size(), MSG_DONTWAIT);

  // results
  if (bytes >= 0)
  {
    value = {bytes, Error::none};
    return Executor::Operation::end;
  }
  else switch (errno) // clang-format off
  {
  case EAGAIN_EWOULDBLOCK:
    this->reactor_->reset_write(this->fd_);
    return Executor::Operation::pri_write;

  case EBADF:
  case EFAULT:
  case EINVAL:
  case EISDIR:
    value.second = Error::invalid_argument;
    break;

  case EINTR:
  case EIO:
    value.second = Error::system_error;
    break;

  defautl:
    value.second = Error::unknown;
    break;
  } // clang-format on

  return Executor::Operation::end;
}
