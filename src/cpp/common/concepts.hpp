#pragma once

#include <cstddef>
#include <span>
namespace concepts
{

template <typename T>
concept ReadNotifiable = requires(T t) { t.notify_read(); };

template <typename T>
concept PriorityReadNotifiable = requires(T t) { t.notify_pri_read(); };

template <typename T>
concept WriteNotifiable = requires(T t) { t.notify_write(); };

template <typename T>
concept CloseNotifiable = requires(T t) { t.notify_close(); };

template <typename T>
concept HalfCloseNotifiable = requires(T t) { t.notify_half_close(); };

template <typename T>
concept ErrorNotifiable = requires(T t) { t.notify_error(); };

template <typename T>
concept Writable = requires(T t, std::span<std::byte> bytes) {
  { t.write(bytes) } -> std::same_as<void>;
};

} // namespace concepts
