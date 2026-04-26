#include "http.hpp"
#include "iterator.hpp"
#include <optional>
#include <sstream>
#include <string>
#include <utility>

namespace protocol
{

using Response = HTTP::Response;
using ParserContext = Response::ParserContext;

Response ParserContext::construct()
{
  if (this->result != ParserResult::Complete) throw "construct an uncompleted request";
  Response response = {version_, status_, headers_, body_};
  return response;
}

void ParserContext::reset()
{
  this->headers_.clear();
  this->result = ParserResult::NeedMoreData;
}

// TODO
ssize_t Response::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  t iterator{_stream};

  std::optional<std::pair<std::string, std::string>> header;
  std::optional<Version> version;
  std::optional<int> status;

  if (_ctx.result == ParserResult::Invalid)
  {
    // throw; ?
  }

  if (_ctx.result == ParserResult::Complete)
  {
    // throw ??
    // reset ??
  }

  switch (_ctx.state_)
  {
  case ParserContext::VERSION:
    if (!iterator.next(' '))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::VERSION;
      return _stream.size() - iterator.head.size();
    }

    version = HTTP::parse_version(iterator.tail);
    if (!version)
    {
      _ctx.result = ParserResult::Invalid;
      return _stream.size() - iterator.head.size();
    }

    _ctx.version_ = version.value();

  case ParserContext::STATUS:
    if (!iterator.next(' '))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::STATUS;
      return _stream.size() - iterator.head.size();
    }

    status = HTTP::parse_status(iterator.tail);
    if (!status)
    {
      _ctx.result = ParserResult::Invalid;
      return _stream.size() - iterator.head.size();
    }

    _ctx.status_ = status.value();

  case ParserContext::STATUS_NAME:
    if (!iterator.next("\r\n"))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::STATUS_NAME;
      return _stream.size() - iterator.head.size();
    }

  case ParserContext::HEADERS:
    if (!iterator.next("\r\n"))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::HEADERS;
      return _stream.size() - iterator.head.size();
    }

    while (!iterator.tail.empty())
    {
      header = HTTP::parse_header(iterator.tail);
      if (!header)
      {
        _ctx.result = ParserResult::Invalid;
        return _stream.size() - iterator.head.size();
      }

      _ctx.headers_.insert(header.value());

      if (!iterator.next("\r\n"))
      {
        _ctx.result = ParserResult::NeedMoreData;
        _ctx.state_ = ParserContext::HEADERS;
        return _stream.size() - iterator.head.size();
      }
    }

  case ParserContext::BODY:
    if (_ctx.headers_.contains("content-length"))
    {
      const char* str = _ctx.headers_["content-length"].c_str();
      unsigned long content_length;
      if (std::sscanf(str, "%lu", &content_length) == EOF)
      {
        _ctx.result = ParserResult::Invalid;
        _ctx.state_ = ParserContext::BODY;
        return _stream.size();
      }

      _ctx.body_ += iterator.head.substr(0, (content_length - _ctx.body_.length()));

      if (_ctx.body_.length() < content_length)
      {
        _ctx.result = ParserResult::NeedMoreData;
        _ctx.state_ = ParserContext::BODY;
        return _stream.size();
      }

      _ctx.result = ParserResult::Complete;
      return _stream.size();
    }

    _ctx.body_ = iterator.head;
    _ctx.result = ParserResult::Complete;
    return _stream.size();

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
