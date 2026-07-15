#pragma once

#include <cstddef>
#include <span>

inline std::span<const char> char_cast(std::span<const std::byte> sp)
{
  return std::span<const char>{reinterpret_cast<const char*>(sp.data()), sp.size()};
}

template <size_t S> consteval std::array<std::byte, S> as_bytes(const char str[S])
{
  std::array<std::byte, S> arr;
  for (auto i = 0; i < S; ++i)
    arr[i] = (std::byte)str[i];

  return arr;
}
