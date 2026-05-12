#pragma once

#include <concepts>
#include <functional>
#include <span>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace protocol
{

enum class ParserResult
{
  NeedMoreData,
  Invalid,
  Complete
};

template <typename T>
concept Parsable = requires(std::span<const char> data, typename T::ParserContext ctx) {
  { T::parse(data, ctx) } -> std::same_as<ssize_t>;
  { ctx.construct() } -> std::same_as<T>;
  { ctx.result } -> std::same_as<ParserResult&>;
  { ctx.error_msg } -> std::same_as<std::string&>;
};

template <typename T>
concept Protocol = requires(typename T::Request request, typename T::Response response) {
  requires Parsable<typename T::Request>;
  requires Parsable<typename T::Response>;
  { std::string(request) } -> std::same_as<std::string>;
  { std::string(response) } -> std::same_as<std::string>;
};

template <Protocol P> using Handler = std::function<typename P::Response(typename P::Request&)>;
template <Protocol P> using Middleware = std::function<typename P::Response(typename P::Request&, Handler<P>)>;

}; // namespace protocol
