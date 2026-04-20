#include "http.hpp"
#include <exception>
#include <regex>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <utility>

namespace protocol
{

struct ParsingException : std::exception
{
  const std::string reason;

  template <typename... Args>
  ParsingException(std::format_string<Args...> _fmt, Args&&... _args)
      : reason{std::format(_fmt, std::forward<Args>(_args)...)} {};

  const char* what() const noexcept override { return this->reason.c_str(); }
};

class Iterator
{
public:
  std::string_view head;
  std::string_view tail;

  Iterator(const std::string_view _str) : head(), tail(_str) {};
  const std::string_view& next(const std::string_view _query)
  {
    const std::size_t p(this->tail.find(_query));
    if (p == std::string_view::npos)
      throw ParsingException("Iterator could not find {} in {}.", _query, this->tail);

    this->head = this->tail.substr(0, p);
    this->tail.remove_prefix(p + _query.size());
    return this->head;
  }
};

HTTP::Request::Request(const std::string_view _s)
{
  Iterator it(_s);
  Iterator cmd(it.next("\r\n"));

  this->method = deserialize_method(cmd.next(" "));
  this->url = cmd.next(" ");
  this->version = deserialize_version(cmd.tail);

  while (it.next("\r\n") != "")
    this->headers.insert(deserialize_header(it.head));

  this->body = it.tail;
}

HTTP::Response::Response(const std::string_view _s)
{
  Iterator it(_s);

  this->version = deserialize_version(it.next(" "));
  this->status = deserialize_status(it.next(" "));
  it.next("\r\n"); // skip status message

  while (it.next("\r\n") != "")
    this->headers.insert(deserialize_header(it.head));

  this->body = it.tail;
}

int HTTP::deserialize_status(const std::string_view _s)
{
  int status = std::stoi(_s.data());
  if (status < 100 || 600 <= status) throw ParsingException("{} isn't a valid http response status.", _s);

  return status;
}

HTTP::Method HTTP::deserialize_method(const std::string_view _s)
{
  // clang-format off
  if (_s == "GET"    ) return HTTP::GET;
  if (_s == "HEAD"   ) return HTTP::HEAD;
  if (_s == "DELETE" ) return HTTP::DELETE;
  if (_s == "CONNECT") return HTTP::CONNECT;
  if (_s == "OPTIONS") return HTTP::OPTIONS;
  if (_s == "TRACE"  ) return HTTP::TRACE;
  if (_s == "PUT"    ) return HTTP::PUT;
  if (_s == "POST"   ) return HTTP::POST;
  // clang-format on

  throw ParsingException("{} isn't a valid http method.", _s);
}

HTTP::Version HTTP::deserialize_version(const std::string_view _s)
{
  if (_s == "HTTP/0.9") return HTTP::Version::HTTP_09;
  if (_s == "HTTP/1.0") return HTTP::Version::HTTP_10;
  if (_s == "HTTP/1.1") return HTTP::Version::HTTP_11;
  if (_s == "HTTP/2.0") return HTTP::Version::HTTP_20;
  if (_s == "HTTP/3.0") return HTTP::Version::HTTP_30;

  throw ParsingException("{} isn't a valid http protocol version.", _s);
}

void remove_ows(std::string& _s)
{
  std::size_t start = _s.find_first_not_of(' ');
  std::size_t end = _s.find_last_not_of(' ');

  if (start == std::string_view::npos || end == std::string_view::npos)
    throw ParsingException("{} is not a valid http header part.", _s);

  _s.erase(0, start);
  _s.erase(end);
}

void to_lower(std::string& _s)
{
  std::transform(_s.begin(), _s.end(), _s.begin(), [](const unsigned char c) { return std::tolower(c); });
}

std::pair<std::string, std::string> HTTP::deserialize_header(const std::string_view _s)
{
  const std::regex reg("[^A-Za-z0-9!#$%&'*+-.^_`|~]");

  const std::size_t colon = _s.find(':');
  if (colon == std::string_view::npos) throw ParsingException("{} is not a valid http header.", _s);

  std::string field(_s.substr(0, colon));
  if (std::regex_search(field, reg)) throw ParsingException("{} field contains invalid character(s).", _s);
  to_lower(field);

  std::string value(_s.substr(colon + 1));
  remove_ows(value);

  return {field, value};
}

} // namespace protocol
