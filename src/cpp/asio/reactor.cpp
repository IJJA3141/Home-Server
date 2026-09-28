#include "reactor.hpp"
#include "../common/assert.hpp"
#include "../common/logger.hpp"

#include <array>
#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

using namespace asio;

void Reactor::Context::update(std::uint32_t events)
{
  this->raw_events |= events;

  this->abs_events.read |= (events & EPOLLIN);
  this->abs_events.write |= (events & EPOLLOUT);
  this->abs_events.closed |= (events & EPOLLHUP);
  this->abs_events.socket |= (events & EPOLLPRI);
  this->abs_events.error |= (events & (EPOLLERR | EPOLLRDHUP));

  return;
}

Reactor::Reactor(std::size_t n) : n_{n}
{
  assert(n > 0);

  const auto& log = Logger::get("Reactor", Logger::stderr);

  if ((this->fd_ = epoll_create1(EPOLL_CLOEXEC)) < 0)
  {
    log.error("creation failed");
    throw std::system_error(errno, std::system_category(), "reactor");
  }

  return;
}

Reactor::~Reactor() noexcept
{
  if (this->fd_ >= 0) this->close();
  return;
}

void Reactor::close() noexcept
{
  assert(this->fd_ >= 0);
  assert(::close(this->fd_));
  this->fd_ = -1;

  return;
}

Reactor::Reactor(Reactor&& other) : fd_{other.fd_}, n_{other.n_}, table_{other.table_}
{
  assert(other.fd_ >= 0);
  assert(other.n_ > 0);

  other.fd_ = -1;

  return;
}

Reactor& Reactor::operator=(Reactor&& other)
{
  assert(other.fd_ >= 0);
  assert(other.n_ > 0);

  if (this->fd_ >= 0) this->close();

  this->fd_ = other.fd_;
  this->n_ = other.n_;
  this->table_ = other.table_;

  other.fd_ = -1;

  return *this;
}

void Reactor::add(int fd, std::uint32_t events)
{
  assert(this->fd_ >= 0);
  assert(!this->has(fd));

  epoll_event event;
  event.data.fd = fd;
  event.events = events | EPOLLET;

  if (epoll_ctl(this->fd_, EPOLL_CTL_ADD, fd, &event))
  {
    const auto& log = Logger::get("Reactor");
    log.error("{}", std::strerror(errno));
    throw std::system_error(errno, std::system_category(), "reactor");
  }

  this->table_[fd] = {};
  return;
}

void Reactor::del(int fd)
{
  assert(this->has(fd));

  if (epoll_ctl(this->fd_, EPOLL_CTL_DEL, fd, nullptr))
  {
    const auto& log = Logger::get("Reactor");
    log.error("{}", std::strerror(errno));
    throw std::system_error(errno, std::system_category(), "reactor");
  }

  this->table_.erase(fd);
  return;
}

void Reactor::run()
{
  assert(this->fd_ >= 0);
  assert(!this->table_.empty());

  epoll_event* events = new epoll_event[this->n_];

  int n = epoll_wait(this->fd_, events, this->n_, -1);
  if (n < 0)
  {
    const auto& log = Logger::get("Reactor", Logger::stderr);
    log.error("epoll wait");
    throw std::system_error(errno, std::system_category(), "reactor");
  }

  for (size_t i = 0; i < n; ++i)
    this->table_[events[i].data.fd].update(events[i].events);

  delete[] events;
  return;
}

void Reactor::reset_read(int fd)
{
  assert(this->has(fd));

  this->table_[fd].abs_events.read = false;
  this->table_[fd].raw_events &= ~EPOLLIN;
}

void Reactor::reset_write(int fd)
{
  assert(this->has(fd));

  this->table_[fd].abs_events.write = false;
  this->table_[fd].raw_events &= ~EPOLLOUT;
}
