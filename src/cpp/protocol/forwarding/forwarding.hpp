#pragma once

#include "../http/http.hpp"
#include "../protocol.hpp"
#include <string>

namespace protocol
{

struct forwarding
{
  using Header = protocol::http::Header;
  using Headers = protocol::http::Headers;

  struct Request
  {
    struct ParserContext;

    std::string host;
    size_t content_length;

    static size_t parse(std::span<const char>, ParserContext&);
  };
};

struct forwarding::Request::ParserContext
{
public:
  ParserResult result;

  ParserContext();

  Request construct();
  void reset();

private:
  enum
  {
    BUFFERING,
    PARSING,
  } state_;

  std::string host_;
  size_t content_length_;
  size_t already_parsed_;

  friend Request;
};

} // namespace protocol
