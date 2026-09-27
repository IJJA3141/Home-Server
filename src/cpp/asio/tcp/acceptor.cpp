#include "../../common/logger.hpp"
#include "tcp.hpp"

#include <cerrno>
#include <expected>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

using namespace asio;
using namespace tcp;

Acceptor::Acceptor(Executor* executor, Reactor* reactor, std::uint32_t addr, std::uint16_t port, std::size_t n)
    : executor_{executor}, reactor_{reactor}
{
  if (executor_ == nullptr) throw std::invalid_argument("nullptr executor");
  if (reactor_ == nullptr) throw std::invalid_argument("nullptr reactor");

  this->addr_.sin_family = AF_INET;
  this->addr_.sin_addr.s_addr = htonl(addr);
  this->addr_.sin_port = htons(port);

  this->fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  if (this->fd_ < 0)
  {
    // err
    throw;
  }

  if (bind(this->fd_, reinterpret_cast<sockaddr*>(&this->addr_), sizeof this->addr_))
  {
    // err
    this->fd_ = -1;
    throw;
  }

  if (listen(this->fd_, n))
  {
    // err
    this->fd_ = -1;
    throw;
  }

  this->executor_->add(this->fd_);
  this->reactor_->add(this->fd_);

  return;
}

void Acceptor::close()
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

Awaitable<std::expected<Socket, Error>> Acceptor::async_accept()
{ // clang-format off
  return {this->fd_, this->executor_, Executor::Operation::read, std::unexpected(Error::none),
    [this](auto& value) { return this->accept(value); }
  }; 
} // clang-format on

Executor::Operation Acceptor::accept(std::expected<Socket, Error>& value)
{
  Reactor::Events events = (*this->reactor_)[this->fd_];
  if(value) return Executor::Operation::end;

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
  else if (events.closed)
  {
    value = std::unexpected(Error::closed);
    return Executor::Operation::end;
  }
  else if (!events.read)
  {
    value = std::unexpected(Error::none);
    return Executor::Operation::pri_read;
  }

  // action
  sockaddr_in addr;
  socklen_t len = sizeof addr;
  int fd = accept4(this->fd_, reinterpret_cast<sockaddr*>(&addr), &len, SOCK_CLOEXEC);

  // results
  if (fd > 0)
  {
    value = Socket{this->executor_, this->reactor_, fd, addr};
    return Executor::Operation::end;
  }
  else switch (errno)
    {
    case EAGAIN_EWOULDBLOCK:
      this->reactor_->reset_read(this->fd_);
      return Executor::Operation::pri_read;

    case EBADF:
    case EFAULT:
    case EINVAL:
    case ENOTSOCK:
    case EOPNOTSUPP:
      value = std::unexpected(Error::invalid_argumnent);
      break;

    case ECONNABORTED:
    case EINTR:
    case EMFILE:
    case ENFILE:
    case ENOBUFS:
    case ENOMEM:
    case EPERM:
    case EPROTO:
      value = std::unexpected(Error::system_error);
      break;

    defautl:
      value = std::unexpected(Error::unknown);
      break;
    }

  return Executor::Operation::end;
}
