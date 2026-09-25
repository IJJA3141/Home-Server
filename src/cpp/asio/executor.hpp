#pragma once

#include <cassert>
#include <coroutine>
#include <functional>
#include <map>
#include <queue>

namespace asio
{

class Executor
{
public:
  enum class Operation
  {
    end = -1,
    pri_read = 0,
    pri_write = 1,
    read = 2,
    write = 3,
  };

  Executor() = default;

  Executor(const Executor&) = delete;
  Executor& operator=(const Executor&) = delete;

  Executor(Executor&&) = default;
  Executor& operator=(Executor&&) = default;

  void add(int fd);
  bool has(int fd) { return this->table_.contains(fd) && !this->table_[fd].dirty; };
  void del(int fd);

  void execute(Operation, int fd, std::function<Operation(void)> executable, std::coroutine_handle<>);
  void run();
  void flush();

private:
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

  std::map<int, node> table_;
  bool executing_;

  std::size_t count_tasks_() const;
};

} // namespace asio
