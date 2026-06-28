#pragma once

#include "../protocol/http/http.hpp"
#include "../protocol/protocol.hpp"
#include <cstddef>
#include <span>

struct ForwardPolicy
{
  size_t parse(std::span<const std::byte>);
  bool wants_handler();

  using Request = http::Request;
  using Response = http::Request;
  Request request;
  Response response;

  std::span<const std::byte> send_buffer();
  void discard(size_t);
  bool release_buffer();

  bool wants_disconnect() { return true; };

private:
  enum class State
  {
    METHOD,
    PATH,
    VERSION,
    HEADERS,
    FORWARDING,

    DONE,
    FAILED,
  } state;

  size_t received;
  size_t forwarded;
};

static_assert(protocol::policy<ForwardPolicy>, "forward should implement policy");
