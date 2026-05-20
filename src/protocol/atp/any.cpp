#include "../../utils/iterator.hpp"
#include "atp.hpp"
#include <optional>
#include <utility>

namespace protocol
{

using Request = ATP::Request;
using Response = ATP::Response;

Request Request::ParserContext::construct()
{
  if (this->result != ParserResult::Complete) throw std::runtime_error("construct an uncompleted request");

  // clang-format off
  switch (this->type_.value())
  {
  case Type::VAL: return {this->val_ctx_.construct()};
  case Type::LOG: return {this->log_ctx_.construct()};
  case Type::GEN: return {this->gen_ctx_.construct()};
  }
  // clang-format on

  std::unreachable();
}

size_t Request::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  if (!_ctx.type_)
  {
    Iterator iterator(_stream);

    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.type_ = parse_type(iterator.head);
    if (!_ctx.type_)
    {
      _ctx.error_msg = "invalid type " + std::string(iterator.head);
      _ctx.result = ParserResult::Invalid;
      return _stream.size() - iterator.tail.size();
    }
  }

  size_t bytes;
  switch (_ctx.type_.value())
  {
  case Type::VAL:
    bytes = ATP::VAL::Request::parse(_stream, _ctx.val_ctx_);
    _ctx.error_msg = _ctx.val_ctx_.error_msg;
    _ctx.result = _ctx.val_ctx_.result;
    break;

  case Type::LOG:
    bytes = ATP::LOG::Request::parse(_stream, _ctx.log_ctx_);
    _ctx.error_msg = _ctx.log_ctx_.error_msg;
    _ctx.result = _ctx.log_ctx_.result;
    break;

  case Type::GEN:
    bytes = ATP::GEN::Request::parse(_stream, _ctx.gen_ctx_);
    _ctx.error_msg = _ctx.gen_ctx_.error_msg;
    _ctx.result = _ctx.gen_ctx_.result;
    break;
  }

  return bytes;
}

Response Response::ParserContext::construct()
{
  if (this->result != ParserResult::Complete) throw std::runtime_error("construct an uncompleted request");

  // clang-format off
  switch (this->type_.value())
  {
  case Type::VAL: return {this->val_ctx_.construct()};
  case Type::LOG: return {this->log_ctx_.construct()};
  case Type::GEN: return {this->val_ctx_.construct()};
  }
  // clang-format on

  std::unreachable();
}

size_t Response::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  if (!_ctx.type_)
  {
    Iterator iterator(_stream);

    if (!iterator.next('\n'))
    {
      _ctx.result = ParserResult::NeedMoreData;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.type_ = parse_type(iterator.head);
    if (!_ctx.type_)
    {
      _ctx.error_msg = "invalid type " + std::string(iterator.head);
      _ctx.result = ParserResult::Invalid;
      return _stream.size() - iterator.tail.size();
    }
  }

  size_t bytes;
  switch (_ctx.type_.value())
  {
  case Type::VAL:
    bytes = ATP::VAL::Response::parse(_stream, _ctx.val_ctx_);
    _ctx.error_msg = _ctx.val_ctx_.error_msg;
    _ctx.result = _ctx.val_ctx_.result;
    break;

  case Type::LOG:
    bytes = ATP::LOG::Response::parse(_stream, _ctx.log_ctx_);
    _ctx.error_msg = _ctx.log_ctx_.error_msg;
    _ctx.result = _ctx.log_ctx_.result;
    break;

  case Type::GEN:
    bytes = ATP::GEN::Response::parse(_stream, _ctx.gen_ctx_);
    _ctx.error_msg = _ctx.gen_ctx_.error_msg;
    _ctx.result = _ctx.gen_ctx_.result;
    break;
  }

  return bytes;
}

} // namespace protocol
