#pragma once

#include "../common/logger.hpp"
#include <concepts>
#include <cstdint>
#include <cstring>
#include <memory>
#include <sys/epoll.h>
#include <unistd.h>

#define REVERSE_PROXY_EPOLL_SIZE 10

using EpollEventHandler = std::function<void(uint32_t)>;

consteval uint32_t merge_events(std::same_as<EPOLL_EVENTS> auto... u) { return (0 | ... | u); }

template <typename T>
concept EpollRegistrable = requires(T registrable) {
  { registrable.socket } -> std::same_as<int&>;
  registrable.error();
  registrable.closed();
};

template <uint32_t flags> std::unique_ptr<EpollEventHandler> generate_handler(EpollRegistrable auto& registrable)
{
  return std::make_unique<EpollEventHandler>([registrable](uint32_t events) mutable {
    if constexpr (flags & EPOLLIN)
      // The associated file is available for read(2) operations.
      if (events & EPOLLIN) registrable.read();

    if constexpr (flags & EPOLLOUT)
      // The associated file is available for write(2) operations.
      if (events & EPOLLOUT) registrable.write();

    if constexpr (flags & EPOLLRDHUP)
      // Stream socket peer closed connection, or shut down writing half of connection.  (This flag is especially
      // useful for writing simple code to detect peer shutdown when using edge-triggered monitoring.)
      if (events & EPOLLRDHUP) registrable.half_closed();

    if constexpr (flags & EPOLLPRI)
      // There is an exceptional condition on the file descriptor.  See the discussion of POLLPRI in poll(2).
      if (events & EPOLLPRI) registrable.pri();

    // Error condition happened on the associated file descriptor.  This event is also reported for the write
    // end of a pipe when the read end has been closed.

    // epoll_wait(2) will always report for this event; it is not necessary to set it in events when calling
    // epoll_ctl().
    if (events & EPOLLERR) registrable.error();

    // Hang up happened on the associated file descriptor.

    // epoll_wait(2) will always wait for this event; it is not necessary to set it in events when calling
    // epoll_ctl().

    // Note that when reading from a channel such as a pipe or a stream socket, this event merely indicates
    // that the peer closed its end of the channel.  Subsequent reads from the channel will return 0 (end of
    // file) only after all outstanding data in the channel has been consumed.
    if (events & EPOLLHUP) registrable.closed();
  });
}

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

  template <uint32_t flags> auto add(EpollRegistrable auto& registrable)
  {
    const auto& log = Logger::get("Epoll", Logger::stderr);
    auto handler = generate_handler<flags>(registrable);

    epoll_event event{flags, epoll_data{.ptr = handler.get()}};
    if (epoll_ctl(this->fd_, EPOLL_CTL_ADD, registrable.socket, &event))
    {
      log.error("epoll_ctl failed (this->fd={})", registrable.socket);
      throw std::system_error(errno, std::system_category(), "epoll ctl");
    }

    return handler;
  }

  void del(EpollRegistrable auto& registrable)
  {
    const auto& log = Logger::get("Epoll", Logger::stderr);

    if (epoll_ctl(this->fd_, EPOLL_CTL_DEL, registrable.socket, nullptr))
    {
      log.error("epoll_ctl failed (this->fd={}, epoll={})", registrable.socket, this->fd_);
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
        (*static_cast<EpollEventHandler*>(event.data.ptr))(event.events);
      }
    }
  }

private:
  int fd_;
};
