#include "../../utils/iterator.hpp"
#include "http.hpp"
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
  if (this->result != ParserResult::Complete) throw std::runtime_error("construct an uncompleted request");
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

size_t Request::parse(std::span<const char> _stream, ParserContext& _ctx)
{
  Iterator iterator{_stream};
  std::optional<HTTP::Method> method;
  std::optional<HTTP::Version> version;
  std::optional<std::pair<std::string, std::string>> header;
  unsigned long content_length = 0;

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
      return _stream.size() - iterator.tail.size();
    }

    method = HTTP::parse_method(iterator.head);
    if (!method)
    {
      _ctx.result = ParserResult::Invalid;
      _ctx.error_msg = "failed to parse method";
      return _stream.size() - iterator.tail.size();
    }

    _ctx.method_ = method.value();

  case ParserContext::PATH:
    if (!iterator.next(' '))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::PATH;
      return _stream.size() - iterator.tail.size();
    }

    _ctx.path_ = iterator.head;

  case ParserContext::VERSION:
    if (!iterator.next("\r\n"))
    {
      _ctx.result = ParserResult::NeedMoreData;
      _ctx.state_ = ParserContext::VERSION;
      return _stream.size() - iterator.tail.size();
    }

    version = HTTP::parse_version(iterator.head);
    if (!version)
    {
      _ctx.result = ParserResult::Invalid;
      _ctx.error_msg = "failed to parse version";
      return _stream.size() - iterator.tail.size();
    }

    _ctx.version_ = version.value();

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
//   case ParserContext::HEADERS:
//     if (!iterator.next("\r\n"))
//     {
//       _ctx.result = ParserResult::NeedMoreData;
//       _ctx.state_ = ParserContext::HEADERS;
//       return _stream.size() - iterator.tail.size();
//     }
//
//     while (!iterator.head.empty())
//     {
//       header = HTTP::parse_header(iterator.head);
//       if (!header)
//       {
//         _ctx.result = ParserResult::Invalid;
//         _ctx.error_msg = "failed to parse headers";
//         return _stream.size() - iterator.tail.size();
//       }
//
//       _ctx.headers_.insert(header.value());
//
//       if (!iterator.next("\r\n"))
//       {
//         _ctx.result = ParserResult::NeedMoreData;
//         _ctx.state_ = ParserContext::HEADERS;
//         return _stream.size() - iterator.tail.size();
//       }
//     }
//
//   case ParserContext::BODY:
//     try
//     {
//       content_length = std::stoul(_ctx.headers_["content-length"]);
//     }
//     catch (std::invalid_argument&)
//     {
//       content_length = 0;
//     }
//     if (content_length <= 0)
//     {
//       _ctx.result = ParserResult::Complete;
//       return _stream.size() - iterator.tail.size();
//     }
//
//     if (content_length < _ctx.body_.size())
//     {
//       _ctx.result = ParserResult::Invalid;
//       _ctx.error_msg = "failed to parse body";
//       return _stream.size() - iterator.tail.size();
//     }
//
//     content_length -= _ctx.body_.size(); // remaining
//     if (content_length <= iterator.tail.size())
//     {
//       _ctx.body_ += iterator.tail.subview(0, content_length);
//       _ctx.result = ParserResult::Complete;
//       return _stream.size() - (iterator.tail.size() - content_length);
//     }
//     else
//     {
//       _ctx.body_ += iterator.tail;
//       _ctx.result = ParserResult::NeedMoreData;
//       _ctx.state_ = ParserContext::BODY;
//       return _stream.size();
//     }
//
//   default:
//     std::unreachable();
//   }
// }

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
