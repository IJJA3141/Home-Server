#pragma once

#include <cstddef>
#include <span>
#include <vector>
namespace ipc
{

class ConnectionBuffer
{
public:
  void* write_begin();
  std::size_t write_length();
  std::span<std::byte> read_span();
  void discard(size_t);

private:
  std::vector<std::byte> buf_;
};

} // namespace ipc
