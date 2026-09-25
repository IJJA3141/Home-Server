#include "reactor.hpp"
#include "../common/logger.hpp"

#include <cstring>
#include <sys/epoll.h>
#include <unistd.h>

using namespace asio;

Reactor::Reactor(std::size_t n) : n_{n}
{
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
  this->close();

  // might add something to notify registered fds

  return;
}

void Reactor::close()
{
  if (this->fd_ < 0) return;

  ::close(this->fd_);
  this->fd_ = -1;

  return;
}

void Reactor::add(int fd, std::uint32_t events)
{
  const auto& log = Logger::get("Reactor");

  if (this->table_.contains(fd))
  {
    log.crit("added (fd={}) but was already in table", fd);
    throw std::invalid_argument("duplicate fd");
  }

  epoll_event event;
  event.data.fd = fd;
  event.events = events | EPOLLET;

  if (epoll_ctl(this->fd_, EPOLL_CTL_ADD, fd, &event))
  {
    log.error("{}", std::strerror(errno));
    throw std::system_error(errno, std::system_category(), "reactor");
  }

  this->table_[fd] = {};
  return;
}

bool Reactor::has(int fd) { return this->table_.contains(fd); }

void Reactor::del(int fd)
{
  const auto& log = Logger::get("Reactor");

  if (!this->table_.contains(fd))
  {
    log.crit("deleted (fd={}) but was not in table", fd);
    throw std::invalid_argument("missing fd");
  }

  if (epoll_ctl(this->fd_, EPOLL_CTL_DEL, fd, nullptr))
  {
    log.error("{}", std::strerror(errno));
    throw std::system_error(errno, std::system_category(), "reactor");
  }

  this->table_.erase(fd);
  return;
}

void Reactor::run()
{
  const auto& log = Logger::get("Reactor", Logger::stderr);
  epoll_event* events = new epoll_event[this->n_];

  int n = epoll_wait(this->fd_, events, this->n_, -1);
  if (n < 0)
  {
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
  this->table_[fd].abs_events.read = false;
  this->table_[fd].raw_events &= ~EPOLLIN;
}

void Reactor::reset_write(int fd)
{
  this->table_[fd].abs_events.write = false;
  this->table_[fd].raw_events &= ~EPOLLOUT;
}
