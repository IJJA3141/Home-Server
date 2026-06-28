#include "../../utils/iterator.hpp"
#include "atp.hpp"
#include <optional>
#include <string>
#include <utility>

namespace protocol
{

using Request = ATP::VAL::Request;
using Response = ATP::VAL::Response;

Request Request::ParserContext::construct()
{
  if (this->result != ParserResult::Complete) throw std::runtime_error("construct an uncompleted request");
  return {source_, id_.value(), ip_, user_agent_};
}

size_t Request::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  Iterator iterator(_stream);

  switch (_ctx.state_)
  {
  case ParserContext::SOURCE:
    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::SOURCE;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.source_ = iterator.head;

  case ParserContext::ID:
    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::ID;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.id_ = Uuid::parse_safe(iterator.head);
    if (!_ctx.id_)
    {
      _ctx.result = ParserResult::Invalid;
      _ctx.error_msg = "failed to parse id";
      return _stream.size() - iterator.tail.size();
    }

  case ParserContext::IP:
    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::IP;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.ip_ = iterator.head;

  case ParserContext::UA:
    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::IP;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.user_agent_ = iterator.head;
    _ctx.result = ParserResult::Complete;
    return _stream.size() - iterator.tail.size();
  }

  std::unreachable();
}

Response Response::ParserContext::construct()
{
  if (this->result != ParserResult::Complete) throw std::runtime_error("construct an uncompleted request");
  return {id_};
}

size_t Response::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  Iterator iterator(_stream);

  if (!iterator.next("\n"))
  {
    _ctx.result = ParserResult::NeedMoreData;
    return _stream.size() - iterator.tail.size();
  }

  auto id = Uuid::parse_safe(iterator.head);
  if (id) _ctx.id_ = id.value();
  else _ctx.id_ = std::unexpected<std::string>{iterator.head};

  _ctx.result = ParserResult::Complete;
  return _stream.size() - iterator.tail.size();
}

} // namespace protocol
