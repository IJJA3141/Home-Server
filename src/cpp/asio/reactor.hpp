#pragma once

#include <cstdint>
#include <map>
#include <sys/epoll.h>

namespace asio
{

class Reactor
{
public:
  // abstract common representation of reactor events (not bound to epoll for future reactors)
  struct Events
  { // clang-format off
    bool read      : 1;
    bool write     : 1;
    bool socket    : 1; // other events like pri_read append on socket
    bool closed    : 1;
    bool error     : 1; // errors like EPOLLERR or EPOLLRDHUP
  }; // clang-format on

  struct Context
  {
    Events abs_events;
    std::uint32_t raw_events;

    void update(std::uint32_t events);
  };

  Reactor(std::size_t n);
  ~Reactor() noexcept;

  void close();

  Reactor(const Reactor&) = delete;
  Reactor& operator=(const Reactor&) = delete;

  Reactor(Reactor&&) = default;
  Reactor& operator=(Reactor&&) = default;

  void add(int fd, std::uint32_t events = EPOLLIN | EPOLLOUT);
  bool has(int fd);
  void del(int fd);

  void run();

  inline Reactor::Events operator[](int fd) const { return this->table_.at(fd).abs_events; }
  inline Reactor::Context& operator()(int fd) { return this->table_[fd]; }

  // used to notify the reactor that EAGAIN or EWOULDWAIT has been return on fd
  void reset_read(int fd);
  void reset_write(int fd);

private:
  int fd_;
  std::size_t n_;
  std::map<int, Context> table_;
};

} // namespace asio
