#include "../../utils/iterator.hpp"
#include "http.hpp"
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace protocol
{

using Response = HTTP::Response;
using ParserContext = Response::ParserContext;

ParserContext::ParserContext() : state_(VERSION), result(ParserResult::NeedMoreData) {}

Response ParserContext::construct()
{
  if (this->result != ParserResult::Complete) throw std::logic_error("construct an uncompleted request");
  Response response = {version_, status_, message_, headers_, body_};
  this->reset();
  return response;
}

void ParserContext::reset()
{
  this->result = ParserResult::NeedMoreData;
  this->headers_.clear();
  this->state_ = VERSION;
}

// TODO
size_t Response::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  Iterator iterator{_stream};

  unsigned long content_length = 0;
  std::optional<std::pair<std::string, std::string>> header;
  std::optional<Version> version;
  std::optional<int> status;

  if (_ctx.result == ParserResult::Invalid) throw std::logic_error("parsed an olready invalid message");
  if (_ctx.result == ParserResult::Complete) throw std::logic_error("parse a completed message");

  switch (_ctx.state_)
  {
  case ParserContext::VERSION:
    if (!iterator.next(' '))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::VERSION;
      return _stream.size() - iterator.tail.size();
    }

    version = HTTP::parse_version(iterator.head);
    if (!version)
    {
      _ctx.result = ParserResult::Invalid;
      _ctx.error_msg = "failed to pares version";
      return _stream.size() - iterator.tail.size();
    }

    _ctx.version_ = version.value();

  case ParserContext::STATUS:
    if (!iterator.next(' '))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::STATUS;
      return _stream.size() - iterator.tail.size();
    }

    status = HTTP::parse_status(iterator.head);
    if (!status)
    {
      _ctx.result = ParserResult::Invalid;
      _ctx.error_msg = "failed to parse status";
      return _stream.size() - iterator.tail.size();
    }

    _ctx.status_ = status.value();

  case ParserContext::STATUS_NAME:
    if (!iterator.next("\r\n"))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::STATUS_NAME;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.message_ = iterator.head;

  case ParserContext::HEADERS:
    if (!iterator.next("\r\n"))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::HEADERS;
      return _stream.size() - iterator.tail.size();
    }

    while (!iterator.head.empty())
    {
      header = HTTP::parse_header(iterator.head);
      if (!header)
      {
        _ctx.result = ParserResult::Invalid;
        _ctx.error_msg = "failed to parse headers";
        return _stream.size() - iterator.tail.size();
      }

      _ctx.headers_.insert(header.value());

      if (!iterator.next("\r\n"))
      {
        _ctx.result = ParserResult::NeedMoreData;
        _ctx.state_ = ParserContext::HEADERS;
        return _stream.size() - iterator.tail.size();
      }
    }

  case ParserContext::BODY:
    if (!_ctx.headers_.contains("content-length"))
    {
      _ctx.result = ParserResult::Complete;
      return _stream.size() - iterator.tail.size();
    }

    try
    {
      content_length = std::stoul(_ctx.headers_["content-length"]);
    }
    catch (std::invalid_argument&)
    {
      content_length = 0;
    }
    if (content_length <= 0)
    {
      _ctx.result = ParserResult::Complete;
      return _stream.size() - iterator.tail.size();
    }

    if (content_length < _ctx.body_.size())
    {
      _ctx.result = ParserResult::Invalid;
      _ctx.error_msg = "failed to parse body";
      return _stream.size() - iterator.tail.size();
    }

    content_length -= _ctx.body_.size(); // remaining
    if (content_length <= iterator.tail.size())
    {
      _ctx.body_ += iterator.tail.subview(0, content_length);
      _ctx.result = ParserResult::Complete;
      return _stream.size() - (iterator.tail.size() - content_length);
    }
    else
    {
      _ctx.body_ += iterator.tail;
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::BODY;
      return _stream.size();
    }

  default:
    std::unreachable();
  }
}

Response::operator std::string() const
{
  std::stringstream ss;

  ss << version_to_string(this->version) << " ";
  ss << status_to_string(this->status) << "\r\n";

  for (const auto& header : this->headers)
    ss << header.first << ": " << header.second << "\r\n";

  ss << "\r\n" << this->body;
  return ss.str();
}

} // namespace protocol
