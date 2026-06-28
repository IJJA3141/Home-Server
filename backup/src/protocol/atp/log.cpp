#include "../../utils/iterator.hpp"
#include "atp.hpp"
#include <expected>
#include <string>
#include <utility>

namespace protocol
{

using Request = ATP::LOG::Request;
using Response = ATP::LOG::Response;

Request Request::ParserContext::construct()
{
  if (this->result != ParserResult::Complete) throw std::runtime_error("construct an uncompleted request");
  return this->request_;
}

size_t Request::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  Iterator iterator(_stream);

  switch (_ctx.state_)
  {
  case ParserContext::USER:
    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::USER;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.request_.user = iterator.head;

  case ParserContext::PWD:
    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::PWD;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.request_.pwd = iterator.head;

  case ParserContext::IP:
    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::IP;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.request_.ip = iterator.head;

  case ParserContext::UA:
    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::IP;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.request_.user_agent = iterator.head;
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

  if (!iterator.next('\n'))
  {
    _ctx.result = ParserResult::NeedMoreData;
    return _stream.size() - iterator.tail.size();
  }

  auto uuid = Uuid::parse_safe(iterator.head);
  if (uuid) _ctx.id_.emplace(uuid.value());
  else _ctx.id_ = std::unexpected<std::string>{iterator.head};

  _ctx.result = ParserResult::Complete;
  return _stream.size() - iterator.tail.size();
}

} // namespace protocol
