#pragma once

#include <cstdint>
#include <sys/epoll.h>
#include <unordered_map>

namespace asio
{

/**
 * @brief Thin wrapper around a Linux epoll instance that turns raw epoll
 *        notifications into an abstract, backend-agnostic event model.
 *
 * A Reactor owns one epoll file descriptor and a table mapping every
 * registered socket fd to its last known event state (Context). The class
 * is designed so that other reactor backends (e.g. kqueue, IOCP) could
 * expose the same public surface while translating into the same abstract
 * Events representation.
 *
 * Scope: the Reactor only tracks and refreshes per-fd event *state* — it
 * does not dispatch callbacks or drive user code. It is meant to be paired
 * with an executor (or equivalent scheduling mechanism) that calls run(),
 * then queries operator[] / operator() for the fds it cares about and
 * decides what to do about them.
 *
 * Triggering mode: fds are registered for edge-triggered notification
 * (EPOLLET). They tell the Reactor "this side of the fd is no longer
 * known-ready" after the caller has drained it until EAGAIN, since
 * edge-triggered epoll will not re-report readiness on its own.
 *
 * Ownership: the epoll file descriptor's lifetime is bound to the Reactor's
 * it is created in the constructor and released exactly once, internally.
 *
 * Thread-safety: none of the methods are synchronized. A single Reactor
 * instance is expected to be driven (run(), add(), del(), ...) from one
 * thread at a time.
 */
class Reactor
{
public:
  // @brief Backend-agnostic summary of what happened on a socket.
  struct Events
  { // clang-format off
    bool read       : 1; // Socket is readable (maps to EPOLLIN).
    bool write      : 1; // Socket is writable (maps to EPOLLOUT).
    bool socket     : 1; // Out-of-band/other socket-level event occurred (e.g. EPOLLPRI).
    bool closed     : 1; // Peer closed the connection / hang-up (EPOLLHUP, EPOLLRDHUP).
    bool error      : 1; // An error condition was reported (EPOLLERR).
  }; // clang-format on

  struct Context
  {
    Events abs_events;
    std::uint32_t raw_events;

    void update(std::uint32_t events);
  };

  /**
   * @brief Create the underlying epoll instance.
   *
   * @param n Maximum number of events in internal buffer used to receive ready events.
   *
   * @throws std::system_error if epoll_create1() fails.
   */
  Reactor(std::size_t n);

  // @brief Releases the epoll file descriptor via close().
  ~Reactor() noexcept;

  // non copyable
  Reactor(const Reactor&) = delete;
  Reactor& operator=(const Reactor&) = delete;

  // movable

  /**
   * @brief Transfers ownership of the epoll fd and registration table.
   *
   * The moved-from Reactor is left in a null state and can safely be
   * destructed.
   */
  Reactor(Reactor&&);

  /**
   * @brief Transfers ownership of the epoll fd and registration table.
   *
   * The moved-from Reactor is left in a null state and can safely be
   * destructed.
   */
  Reactor& operator=(Reactor&&);

  /**
   * @brief Register a socket with the epoll instance.
   *
   * @pre fd >= 0 && has(fd) == false.
   *
   * @param fd File descriptor to watch.
   * @param events Raw epoll interest mask (EPOLLET is always on).
   *
   * @throws std::system_error if the underlying epoll_ctl(ADD) fails
   */
  void add(int fd, std::uint32_t events = EPOLLIN | EPOLLOUT);

  /**
   * @brief Check whether fd is currently registered with this reactor.
   * @return true if fd has an entry in the internal table.
   */
  [[nodiscard]] inline bool has(int fd) const { return this->table_.contains(fd); };

  /**
   * @brief Deregister a socket from the epoll instance and drop its Context.
   *
   * @pre has(fd)
   *
   * @param fd Registered file descriptor.
   *
   * @throws std::system_error if the underlying epoll_ctl(DEL) fails.
   */
  void del(int fd);

  /**
   * @brief Block on epoll_wait() and refresh the Context of every fd that
   *        reported activity.
   *
   * Retrieves at most n (see constructor) ready events per call and refreshes
   * the corresponding entries in the internal table. It does not invoke any
   * user code; the caller (executor) inspects operator[] / operator()
   * afterwards for the fds it is interested in and reacts itself.
   *
   * @throws std::system_error if epoll_wait() fails for a reason other than
   * EINTR (which is retried internally).
   */
  void run();

  /**
   * @brief Read-only access to the abstract event state of a registered fd.
   *
   * @param fd Registered file descriptor.
   *
   * @throws std::out_of_range if has(fd) == false.
   */
  inline Reactor::Events operator[](int fd) const { return this->table_.at(fd).abs_events; }

  /**
   * @brief Mutable access to the full Context of fd.
   *
   * @param fd Registered file descriptor.
   *
   * @throws std::out_of_range if has(fd) == false.
   */
  inline Reactor::Context& operator()(int fd) { return this->table_.at(fd); }

  /**
   * @brief Notify the reactor that a read on fd returned EAGAIN/EWOULDBLOCK.
   *
   * fs's readable state is cleared until edge-triggered epoll reports the fd ready again.
   *
   * @pre has(fd)
   */
  void reset_read(int fd);

  /**
   * @brief Notify the reactor that a write on fd returned EAGAIN/EWOULDBLOCK.
   *
   * fs's readable state is cleared until edge-triggered epoll reports the fd ready again.
   *
   * @pre has(fd)
   */
  void reset_write(int fd);

private:
  int fd_;
  std::size_t n_;
  std::unordered_map<int, Context> table_;

  void close() noexcept;
};

static_assert(std::movable<Reactor>, "Reactor should be movable");
static_assert(!std::copyable<Reactor>, "Reactor shouldn't be copyable");

} // namespace asio
