#include "../../common/iterator.hpp"
#include "forwarding.hpp"
#include <cassert>
#include <stdexcept>

namespace protocol
{

using Request = forwarding::Request;
using ParserContext = forwarding::Request::ParserContext;

ParserContext::ParserContext() : result{ParserResult::NeedMoreData}, state_{BUFFERING} {}

Request ParserContext::construct()
{
  assert(this->result == ParserResult::Complete);
  return Request{this->host_, this->content_length_};
}

void ParserContext::reset()
{
  this->result = ParserResult::NeedMoreData;
  this->state_ = BUFFERING;
  this->host_.clear();
  this->content_length_ = 0;
  this->already_parsed_ = 0;
}

size_t forwarding::Request::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  assert(_ctx.result == ParserResult::NeedMoreData);

  Iterator iterator{_stream.subspan(_ctx.already_parsed_)};

  switch (_ctx.state_)
  {
  case ParserContext::BUFFERING:
    if (!iterator.next("\r\n")) break;

    // ignore start line (no use for reverse proxy {for now})
    _ctx.state_ = ParserContext::PARSING;
    [[fallthrough]];

  case ParserContext::PARSING:
    if (!iterator.next("\r\n")) break;

    while (!iterator.head.empty())
    { // at least one header
      auto header = protocol::http::parse_header(iterator.head);
      if (!header)
      { // invalid request
        _ctx.result = ParserResult::Invalid;
        break;
      }

      if (header->first == "host") _ctx.host_ = std::move(header->second);
      else if (header->first == "content-length") try
        {
          _ctx.content_length_ = std::stoul(header->second);
        }
        catch (std::invalid_argument&)
        {
          _ctx.content_length_ = 0;
        }

      if (!iterator.next("\r\n")) break;
    }

    _ctx.result = ParserResult::Complete;
  }

  return _ctx.already_parsed_ = _stream.size() - iterator.tail.size();
}

} // namespace protocol
