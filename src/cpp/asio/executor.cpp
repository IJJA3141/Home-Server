#include "executor.hpp"
#include "../common/logger.hpp"

#include <stdexcept>
#include <utility>

using namespace asio;

void Executor::add(int fd)
{
  if (this->table_.contains(fd) && !this->table_[fd].dirty)
  {
    const auto log = Logger::get("Executor");
    log.crit("added (fd={}) but was already in table", fd);
    throw std::invalid_argument("duplicate fd");
  }

  this->table_[fd] = {};
  return;
}

void Executor::del(int fd)
{
  if (!this->table_.contains(fd))
  {
    const auto log = Logger::get("Executor");
    log.crit("deleted (fd={}) but was not in table", fd);
    throw std::invalid_argument("missing fd");
  }

  if (this->executing_) this->table_[fd].dirty = true;
  else this->table_.erase(fd);

  return;
}

void Executor::execute(Operation op, int fd, std::function<Operation(void)> executable,
                       std::coroutine_handle<> handle)
{
  const auto log = Logger::get("Executor");

  if (!this->table_.contains(fd))
  {
    log.crit("executed action for (fd={}) but fd was not in table", fd);
    throw std::invalid_argument("missing fd");
  }

  if (this->table_[fd].dirty) log.warn("executed new task on dirty (fd={})", fd);
  this->table_[fd][op].push({handle, executable});
  return;
}

void Executor::run()
{
  const auto log = Logger::get("Executor");
  log.info("starting execution with {} tasks.", this->count_tasks_());

  this->executing_ = true;
  for (auto& [fd, node] : this->table_)
  {
    for (int i = 0; i < 4; ++i)
    {
      auto& task_queue = node[static_cast<Operation>(i)];
      if (!task_queue.empty())
      {
        auto& task = task_queue.front();
        task_queue.pop();

        Operation op = task.executable();
        if (op != Operation::end)
        {
          node[op].push(task);
          break;
        }

        task.handle.resume();
      }
    }
  }
  this->executing_ = false;
  this->flush();

  log.info("ending execution with {} tasks.", this->count_tasks_());
  return;
}

void Executor::flush()
{
  if (this->executing_)
  {
    const auto log = Logger::get("Executor");
    log.crit("flushed while running");
    throw std::runtime_error("flush during run");
  }

  std::erase_if(this->table_, [](node& val) { return val.dirty; });
}

std::size_t Executor::count_tasks_() const
{
  std::size_t n = 0;

  for (const auto& [_, node] : this->table_)
  {
    n += node.pri_read.size();
    n += node.pri_write.size();
    n += node.read.size();
    n += node.write.size();
  }

  return n;
}

std::queue<Executor::node::task>& Executor::node::operator[](Operation op)
{
  switch (op)
  { // clang-format off
  case Operation::pri_read:   return this->pri_read;
  case Operation::pri_write:  return this->pri_write;
  case Operation::read:       return this->read;
  case Operation::write:      return this->write;
  default:
    const auto log = Logger::get("Executor");
    log.crit("out of bounds (op={})", static_cast<int>(op));
    throw std::invalid_argument(std::to_string(static_cast<int>(op)) + " out of bounds");
  } // clang-format on

  std::unreachable();
}
