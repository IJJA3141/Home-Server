#pragma once

#include "../common/concepts.hpp"
#include "../common/logger.hpp"
#include <concepts>
#include <cstdint>
#include <cstring>
#include <memory>
#include <sys/epoll.h>
#include <unistd.h>

namespace epoll
{

using EventHandler = std::function<void(uint32_t)>;

template <typename T, uint32_t F>
concept ReadRegistable = (!(F & EPOLLIN) || concepts::ReadNotifiable<T>);

template <typename T, uint32_t F>
concept WriteRegistable = (!(F & EPOLLOUT) || concepts::WriteNotifiable<T>);

template <typename T, uint32_t F>
concept HalfCloseRegistrable = (!(F & EPOLLRDHUP) || concepts::HalfCloseNotifiable<T>);

template <typename T, uint32_t F>
concept PriorityReadRegistrable = (!(F & EPOLLPRI) || concepts::PriorityReadNotifiable<T>);

template <typename T>
concept HasFd = requires(T t) {
  { t.fd() } -> std::same_as<int>;
};

// clang-format off
template <typename T, uint32_t F>
concept Registrable = ReadRegistable<T, F> &&
                      WriteRegistable<T, F> &&
                      HalfCloseRegistrable<T, F> &&
                      PriorityReadRegistrable<T, F> &&
                      concepts::CloseNotifiable<T> && concepts::ErrorNotifiable<T>
                      && HasFd<T>;
// clang-format on

template <uint32_t flags> EventHandler* generate_handler(Registrable<flags> auto& registrable)
{
  return new EventHandler([registrable](uint32_t events) mutable {
    // There is an exceptional condition on the file descriptor.  See the
    // discussion of POLLPRI in poll(2).

    // pri stands for priority
    if constexpr (flags & EPOLLPRI)
      if (events & EPOLLPRI) registrable.notify_pri_read();

    // The associated file is available for read(2) operations.
    if constexpr (flags & EPOLLIN)
      if (events & EPOLLIN) registrable.notify_read();

    // The associated file is available for write(2) operations.
    if constexpr (flags & EPOLLOUT)
      if (events & EPOLLOUT) registrable.notify_write();

    // Stream socket peer closed connection, or shut down writing half of
    // connection.  (This flag is especially useful for writing simple code
    // to detect peer shutdown when using edge-triggered monitoring.)
    if constexpr (flags & EPOLLRDHUP)
      if (events & EPOLLRDHUP) registrable.notify_half_close();

    // Error condition happened on the associated file descriptor.  This event
    // is also reported for the write end of a pipe when the read end has been
    // closed.

    // epoll_wait(2) will always report for this event; it is not necessary to
    // set it in events when calling epoll_ctl().
    if (events & EPOLLERR) registrable.notify_error();

    // Hang up happened on the associated file descriptor.

    // epoll_wait(2) will always wait for this event; it is not necessary to
    // set it in events when calling epoll_ctl().

    // Note that when reading from a channel such as a pipe or a stream socket,
    // this event merely indicates that the peer closed its end of the channel.
    // Subsequent reads from the channel will return 0 (end of file) only after
    // all outstanding data in the channel has been consumed.
    if (events & EPOLLHUP) registrable.notify_close();
  });
}

} // namespace epoll

class Epoll
{
public:
  Epoll()
  {
    const auto& log = Logger::get("Epoll", Logger::stderr);

    if ((this->fd_ = epoll_create1(0)) < 0)
    {
      log.crit("creation failed");
      throw std::system_error(errno, std::system_category(), "epoll");
    }
  }

  ~Epoll() noexcept { close(this->fd_); }

  template <uint32_t flags> auto add(epoll::Registrable<flags> auto& registrable)
  {
    const auto& log = Logger::get("Epoll", Logger::stderr);
    epoll::EventHandler* handler = epoll::generate_handler<flags>(registrable);

    epoll_event event{flags, epoll_data{.ptr = handler}};
    if (epoll_ctl(this->fd_, EPOLL_CTL_ADD, registrable.fd(), &event))
    {
      log.error("epoll_ctl failed (this->fd={})", registrable.fd());
      throw std::system_error(errno, std::system_category(), "epoll ctl");
    }

    return std::unique_ptr<epoll::EventHandler>(handler);
  }

  void del(epoll::HasFd auto& registrable)
  {
    const auto& log = Logger::get("Epoll", Logger::stderr);

    if (epoll_ctl(this->fd_, EPOLL_CTL_DEL, registrable.fd(), nullptr))
    {
      log.error("epoll_ctl failed (this->fd={}, epoll={})", registrable.fd(), this->fd_);
      throw std::system_error(errno, std::system_category(), "epoll ctl");
    }
  }

  template <size_t S> void start()
  {
    const auto& log = Logger::get("Epoll", Logger::stderr);

    while (true)
    {
      epoll_event events[S];
      int n = epoll_wait(this->fd_, events, S, -1);
      if (n < 0)
      {
        log.crit("epoll wait failed");
        throw std::system_error(errno, std::system_category(), "epoll wait");
      }

      log.debug("received {} events", n);
      for (size_t i = 0; i < n; ++i)
      {
        epoll_event event = events[i];
        (*static_cast<epoll::EventHandler*>(event.data.ptr))(event.events);
      }
    }
  }

private:
  int fd_;
};
