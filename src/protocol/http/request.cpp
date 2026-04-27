#include "http.hpp"
#include "iterator.hpp"
#include <cstdio>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace protocol
{

using Request = HTTP::Request;
using ParserContext = Request::ParserContext;

ParserContext::ParserContext() : state_(METHOD), result(ParserResult::NeedMoreData) {}

Request ParserContext::construct()
{
  if (this->result != ParserResult::Complete) throw "construct an uncompleted request";
  Request request = {method_, path_, version_, headers_, body_};
  // resset ??
  return request;
}

void ParserContext::reset()
{
  this->headers_.clear();
  this->result = ParserResult::NeedMoreData;
  this->state_ = METHOD;
}

ssize_t Request::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  t iterator{_stream};
  std::optional<HTTP::Method> method;
  std::optional<HTTP::Version> version;
  std::optional<std::pair<std::string, std::string>> header;

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
  case ParserContext::METHOD:
    if (!iterator.next(' '))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::METHOD;
      return _stream.size() - iterator.head.size();
    }

    method = HTTP::parse_method(iterator.tail);
    if (!method)
    {
      _ctx.result = ParserResult::Invalid;
      return _stream.size() - iterator.head.size();
    }

    _ctx.method_ = method.value();

  case ParserContext::PATH:
    if (!iterator.next(' '))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::PATH;
      return _stream.size() - iterator.head.size();
    }

    _ctx.path_ = iterator.tail;

  case ParserContext::VERSION:
    if (!iterator.next("\r\n"))
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

    _ctx.result = ParserResult::Complete;
    return _stream.size() - iterator.head.size();

  default:
    std::unreachable();
  }
}

Request::operator std::string() const
{
  std::stringstream ss;
  ss << method_to_string(this->method) << " " << this->path;
  ss << " " << version_to_string(this->version) << "\r\n";

  for (const auto& header : this->headers)
    ss << header.first << ": " << header.second << "\r\n";

  ss << "\r\n" << this->body;

  return ss.str();
}

} // namespace protocol
