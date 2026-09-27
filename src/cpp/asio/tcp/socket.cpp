#include "../../common/logger.hpp"
#include "tcp.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <expected>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

using namespace asio;
using namespace tcp;

Socket::Socket(Executor* executor, Reactor* reactor, int fd, sockaddr_in addr)
    : addr_{addr}, SocketBase{executor, reactor, fd}
{
  if (reactor == nullptr) throw std::invalid_argument("nullptr reactor");
}

void Socket::close()
{
  if (this->fd_ < 0) return;

  if (::close(this->fd_))
  {
    auto log = Logger::get("");

    switch (errno)
    {
    case EBADF:
      log.error("(fd={}) argument is not a open file descriptor", this->fd_);
      break;

    case EINTR: // The close() function was interrupted by a signal.
    case EIO:   // An I/O error occurred while reading from or writing to the file system.
    defautl:
      log.warn("first attempt at closing (fd={}) was ont successful", this->fd_);
      if (::close(this->fd_)) log.error("second attempt at closing (fd={}) was ont successful", this->fd_);
    }
  }

  this->reactor_->del(this->fd_);
  this->executor_->del(this->fd_);
  this->fd_ = -1;

  return;
}

Awaitable<std::expected<Socket, Error>> Socket::async_connect(Executor* executor, Reactor* reactor,
                                                              std::span<const char, INET_ADDRSTRLEN> address,
                                                              std::uint16_t port)
{
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  if (inet_aton(address.data(), &addr.sin_addr) == 0)
  {
    // err
    throw;
  }
  addr.sin_port = htons(port);

  int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (fd < 0)
  {
    // err
    throw;
  }

  // clang-format off
  return {fd, executor, Executor::Operation::read, Socket{executor, reactor, fd, addr}, 
    [](auto& value) 
    {
      assert(value); // cannot be unexpected value
      Reactor::Events events = (*value->reactor_)[value->fd_];

      // conditions
      if (events.error)
      {
        value = std::unexpected(Error::system_error);
        return Executor::Operation::end;
      }
      else if (events.socket)
      {
        value = std::unexpected(Error::socket_event);
        return Executor::Operation::end;
      }
      else if (!events.read)
      {
        return Executor::Operation::pri_read;
      }

      socklen_t len = sizeof value->addr_;
      int res = connect(value->fd_, reinterpret_cast<sockaddr*>(&value->addr_), len);

      if (res == -1) switch (errno)
        {
        case EAGAIN_EWOULDBLOCK:
          return Executor::Operation::pri_read;

        case EACCES:
          break;

        defautl:
          value = std::unexpected(Error::unknown);
          break;
        }

      return Executor::Operation::end;
    }
  };
}
// clang-format on

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
    value.second = Error::invalid_argumnent;
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
    value.second = Error::invalid_argumnent;
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
