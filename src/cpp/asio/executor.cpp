#include "executor.hpp"
#include "../common/assert.hpp"
#include "../common/logger.hpp"

#include <utility>

using namespace asio;

Executor::Executor(Executor&& other) : table_{other.table_}, executing_{other.executing_}
{
  other.table_.clear(); // make sure to remove handle from other

  return;
}

Executor& Executor::operator=(Executor&& other)
{
  this->table_ = other.table_;
  this->executing_ = other.executing_;
  other.table_.clear();

  return *this;
}

void Executor::add(int fd)
{
  assert(!this->has(fd));

  // default should be good
  this->table_[fd] = {};
  return;
}

void Executor::del(int fd)
{
  assert(this->has(fd));

  if (this->executing_) this->table_[fd].dirty = true;
  else this->table_.erase(fd);

  return;
}

void Executor::execute(Operation op, int fd, std::function<Operation(void)> executable,
                       std::coroutine_handle<> handle)
{
  assert(this->has(fd));
  assert(op != Operation::end);
  assert(executable);

  this->table_[fd][op].push({handle, executable});
  return;
}

void Executor::run()
{
  assert(!this->executing_);

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
  assert(!this->executing_);
  std::erase_if(this->table_, [](auto& val) { return val.second.dirty; });
  return;
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
    assert(false, "invalid op (op={})", static_cast<int>(op));
  } // clang-format on
}
