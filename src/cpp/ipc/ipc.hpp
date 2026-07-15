#pragma once

#include <cstddef>
#include <span>

namespace ipc
{

struct IClient
{
  virtual size_t send(std::span<const std::byte>) = 0;
  virtual std::span<std::byte> recv() = 0;
};

struct IServer
{
  virtual void tmp() = 0;
};

} // namespace ipc
