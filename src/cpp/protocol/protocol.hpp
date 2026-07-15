#pragma once

#include <concepts>
#include <cstddef>
#include <functional>
#include <span>
#include <sys/socket.h>
#include <utility>

namespace protocol
{

enum class ParserResult
{
  NeedMoreData,
  Invalid,
  Complete
};

template <class P>
concept policy = requires(P policy) {
  typename P::Request;
  typename P::Response;
  { policy.request } -> std::same_as<typename P::Request&>;
  { policy.response } -> std::same_as<typename P::Response&>;

  { policy.parse(std::declval<std::span<const std::byte>>()) } -> std::same_as<std::size_t>;
  { policy.wants_handler() } -> std::same_as<bool>;

  { policy.send_buffer() } -> std::same_as<std::span<const std::byte>>;
  { policy.discard(std::declval<std::size_t>()) };
  { policy.release_buffer() } -> std::same_as<bool>;
  { policy.wants_disconnect() } -> std::same_as<bool>;
};

template <typename P> using Handler = std::function<void(typename P::Request&, typename P::Response& b)>;

} // namespace protocol
