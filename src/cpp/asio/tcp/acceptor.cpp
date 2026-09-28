#include "../../common/assert.hpp"
#include "tcp.hpp"

#include <cerrno>
#include <expected>
#include <sys/socket.h>
#include <unistd.h>

using namespace asio;
using namespace tcp;

Acceptor::Acceptor(Executor* executor, Reactor* reactor, std::uint32_t addr, std::uint16_t port, std::size_t n)
    : executor_{executor}, reactor_{reactor}
{
  assert(executor_ != nullptr);
  assert(reactor_ != nullptr);

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

Acceptor::~Acceptor() noexcept
{
  if (this->executor_ == nullptr && this->reactor_ == nullptr && this->fd_ < 0) return;
  this->close();
  return;
}

void Acceptor::close() noexcept
{
  assert(this->executor_ != nullptr);
  assert(this->reactor_ != nullptr);
  assert(this->fd_ >= 0);
}

Acceptor::Acceptor(Acceptor&& other) : executor_{other.executor_}, reactor_{other.reactor_}, fd_{other.fd_}
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
}

Acceptor& Acceptor::operator=(Acceptor&& other)
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

Awaitable<std::expected<Socket, Error>> Acceptor::async_accept() const
{ // clang-format off
  assert(this->fd_ >= 0);

  return {this->fd_, this->executor_, Executor::Operation::read, std::unexpected(Error::none),
    [this](auto& value) { return this->accept(value); }
  }; 
} // clang-format on

Executor::Operation Acceptor::accept(std::expected<Socket, Error>& value) const
{
  Reactor::Events events = (*this->reactor_)[this->fd_];
  if (value) return Executor::Operation::end;

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
      value = std::unexpected(Error::invalid_argument);
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
