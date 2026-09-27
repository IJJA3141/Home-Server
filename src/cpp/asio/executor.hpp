#pragma once

#include <concepts>
#include <coroutine>
#include <functional>
#include <queue>
#include <unordered_map>

namespace asio
{

// TODO might change the comments
// TODO might be copyable

/**
 * @brief Per-fd, per-operation task queue driving coroutine execution once
 *        readiness is known.
 *
 * An Executor holds four priority-ordered task queues (pri_read, pri_write,
 * read, write) per registered file descriptor. It performs no I/O itself
 * and has no notion of readiness it is meant to be paired with a Reactor (or
 * equivalent) that determines, per fd, which operations are currently
 * ready; run() then dequeues and resumes the matching tasks.
 *
 * Deferred deletion: because run() iterates the registration table while
 * executing arbitrary user coroutines — which may themselves call add() or
 * del() reentrantly — del() cannot safely erase an entry mid-iteration.
 * Instead it marks the fd's node dirty; has() treats dirty fds as absent,
 * and the actual erase happens later in flush().
 *
 * Ownership: the Executor does not own the coroutine handles it stores —
 * it only uses them to resume tasks once their executable reports
 * Operation::end. Whoever holds a coroutine's own handle (typically the
 * code that owns/awaits it, outside this class) is responsible for its
 * lifetime. In particular, any tasks still queued for an fd when del()/
 * flush() discards them are dropped as-is, without being resumed or
 * destroyed — the caller must already have decided what happens to those
 * handles (destroy them, force-resume them, migrate them to another
 * Executor, ...) through its own bookkeeping, independently of this call.
 *
 * Thread-safety: none of the methods are synchronized. A single Executor
 * is expected to be driven (run(), add(), del(), execute(), ...) from one
 * thread at a time.
 */
class Executor
{
public:
  // @brief Executable type to say what task wants next.
  enum class Operation
  {
    end = -1,
    pri_read = 0,
    pri_write = 1,
    read = 2,
    write = 3,
  };

  // @brief Trivially constructable.
  Executor() = default;

  // non copyable
  Executor(const Executor&) = delete;
  Executor& operator=(const Executor&) = delete;

  // movable

  /**
   * @brief Transfers the registration table
   *
   * Every queue/coroutine handle it holds to the new instance. The moved-from
   * Executor is left empty and must not be used except to be destroyed or
   * move-assigned to.
   */
  Executor(Executor&&);

  /**
   * @brief Transfers the registration table
   *
   * Every queue/coroutine handle it holds to the new instance. The moved-from
   * Executor is left empty and must not be used except to be destroyed or
   * move-assigned to.
   */
  Executor& operator=(Executor&&);

  /**
   * @brief Register fd for task scheduling.
   * @param fd Unregistered file descriptor.
   * @pre has(fd) == false
   */
  void add(int fd);

  /**
   * @brief Check whether fd is registered and not pending removal.
   * @param fd File descriptor.
   * @return true iff fd has a live (non-dirty) entry in the table.
   */
  [[nodiscard]] inline bool has(int fd) const { return this->table_.contains(fd) && !this->table_.at(fd).dirty; };

  /**
   * @brief Mark fd for removal.
   *
   * Does not erase the node or resume/destroy any coroutine handles still
   * queued for fd immediately. The node is dropped, as-is, later in flush().
   * Any such handles are the caller's responsibility, not this class's. After
   * this call, has(fd) reports false even though the entry is still physically
   * present until flushed.
   *
   * @pre has(fd)
   * @param fd Registered file descriptor.
   */
  void del(int fd);

  /**
   * @brief Enqueue a coroutine task onto one of fd's queues.
   *
   * @pre has(fd) and operation != Operation::end.
   *
   * @param operation Which of fd's four queues to enqueue onto.
   * @param fd Target file descriptor; must already be registered.
   * @param executable Invoked by run() when the task is attempted.
   * @param handle Coroutine handle resumed by run() once executable returns
   * Operation::end.
   */
  void execute(Operation, int fd, std::function<Operation(void)> executable, std::coroutine_handle<>);

  /**
   * @brief Iterate every registered fd and drive whichever queued tasks
   *        are attempted this pass.
   *
   * For each task popped from a queue: invokes executable(); if it returns
   * Operation::end, resumes the task's coroutine handle (the awaited
   * operation is satisfied); otherwise re-enqueues {handle, executable}
   * onto the queue selected by the returned Operation.
   *
   * Sets executing_ for the duration of the call so that add()/del()
   * invoked reentrantly from within a task's own executable are deferred
   * safely via the dirty-flag mechanism (see del()) instead of mutating
   * table_ while it is being iterated.
   */
  void run();

  /**
   * @brief Erase every node marked dirty by a prior del().
   *
   * Discarded tasks are simply dropped. Their coroutine handles are
   * neither resumed nor destroyed here (see class-level Ownership note).
   * Must not be called while executing_ is true (i.e. reentrantly from
   * within run()), since that would erase entries run() is still
   * iterating. Intended to be called once run() has finished a pass —
   * candidate for being folded into run() and made private, as the
   * original declaration comment already suggested.
   */
  void flush();

private:
  // @brief Per-fd storage: one task queue per Operation, plus a dirty flag
  // used for deferred deletion (see Executor::del()).
  struct node
  {
    struct task
    {
      std::coroutine_handle<> handle;
      std::function<Operation(void)> executable;
    };

    bool dirty = false;
    std::queue<task> pri_read = {};
    std::queue<task> pri_write = {};
    std::queue<task> read = {};
    std::queue<task> write = {};

    [[nodiscard]] std::queue<task>& operator[](Operation);
  };

  std::unordered_map<int, node> table_ = {};
  bool executing_ = false;

  // @brief Total number of tasks currently queued across every registered fd
  // and every operation.
  std::size_t count_tasks_() const;
};

static_assert(std::movable<Executor>, "Executor should be movable");
static_assert(!std::copyable<Executor>, "Executor shouldn't be copyable");

} // namespace asio
