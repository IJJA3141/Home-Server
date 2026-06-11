#include "http.hpp"
#include "../../utils/iterator.hpp"
#include <optional>
#include <regex>
#include <stdexcept>

namespace protocol
{

HTTP::Response HTTP::standard_response(int status) { return {HTTP::Version::HTTP_11, status, "", {}, ""}; }

std::optional<HTTP::Version> HTTP::parse_version(std::string_view _)
{
  if (_ == "HTTP/0.9") return HTTP::Version::HTTP_09;
  if (_ == "HTTP/1.0") return HTTP::Version::HTTP_10;
  if (_ == "HTTP/1.1") return HTTP::Version::HTTP_11;
  if (_ == "HTTP/2.0") return HTTP::Version::HTTP_20;
  if (_ == "HTTP/3.0") return HTTP::Version::HTTP_30;
  return std::nullopt;
}

std::optional<int> HTTP::parse_status(std::string_view _)
{
  int status;
  try
  {
    status = std::stoi(_.data());
  }
  catch (std::invalid_argument&)
  {
    return std::nullopt;
  }
  if (100 <= status || status < 600) return status;
  return std::nullopt;
}

std::optional<HTTP::Method> HTTP::parse_method(std::string_view _)
{ // clang-format off
  if (_ == "GET"    ) return HTTP::Method::GET;
  if (_ == "HEAD"   ) return HTTP::Method::HEAD;
  if (_ == "DELETE" ) return HTTP::Method::DELETE;
  if (_ == "CONNECT") return HTTP::Method::CONNECT;
  if (_ == "OPTIONS") return HTTP::Method::OPTIONS;
  if (_ == "TRACE"  ) return HTTP::Method::TRACE;
  if (_ == "PUT"    ) return HTTP::Method::PUT;
  if (_ == "POST"   ) return HTTP::Method::POST;
  return std::nullopt;
} // clang-format on

bool remove_ows(std::string& _)
{
  std::size_t start = _.find_first_not_of(' ');
  std::size_t end = _.find_last_not_of(' ');

  if (start == std::string::npos || end == std::string::npos) return false;

  _.erase(0, start);
  _.erase(end);
  return true;
}

inline void to_lower(std::string& _)
{
  std::transform(_.begin(), _.end(), _.begin(), [](const unsigned char c) { return std::tolower(c); });
}

std::optional<std::pair<std::string, std::string>> HTTP::parse_header(std::string_view _)
{
  const std::regex reg("[^A-Za-z0-9!#$%&'*+-.^_`|~]");

  const std::size_t colon = _.find(':');
  if (colon == std::string_view::npos) return std::nullopt;

  std::string field(_.substr(0, colon));
  if (std::regex_search(field, reg)) return std::nullopt;
  to_lower(field);

  std::string value(_.substr(colon + 1));

  if (remove_ows(value)) return std::pair(field, value);
  return std::nullopt;
}

std::string HTTP::version_to_string(HTTP::Version _)
{
  switch (_)
  { // clang-format off
  case HTTP::Version::HTTP_09: return "HTTP/0.9";
  case HTTP::Version::HTTP_10: return "HTTP/1.0";
  case HTTP::Version::HTTP_11: return "HTTP/1.1";
  case HTTP::Version::HTTP_20: return "HTTP/2.0";
  case HTTP::Version::HTTP_30: return "HTTP/3.0";
  default: std::unreachable();
  } // clang-format on
}
std::string HTTP::status_to_string(int _)
{
  switch (_)
  { // clang-format off
  case 100: return "100 Continue";
  case 101: return "101 Switching Protocols";
  case 103: return "103 Early Hints";

  case 200: return "200 OK";
  case 201: return "201 Created";
  case 202: return "202 Accepted";
  case 203: return "203 Non-Authoritative Information";
  case 204: return "204 No Content";
  case 205: return "205 Reset Content";
  case 206: return "206 Partial Content";
  case 226: return "226 IM Used";

  case 300: return "300 Multiple Choices";
  case 301: return "301 Moved Permanently";
  case 302: return "302 Found";
  case 303: return "303 See Other";
  case 304: return "304 Not Modified";
  case 307: return "307 Temporary Redirect";
  case 308: return "308 Permanent Redirect";

  case 400: return "400 Bad Request";
  case 401: return "401 Unauthorized";
  case 403: return "403 Forbidden";
  case 404: return "404 Not Found";
  case 405: return "405 Method Not Allowed";
  case 406: return "406 Not Acceptable";
  case 407: return "407 Proxy Authentication Required";
  case 408: return "408 Request Timeout";
  case 409: return "409 Conflict";
  case 410: return "410 Gone";
  case 411: return "411 Length Required";
  case 412: return "412 Precondition Failed";
  case 413: return "413 Payload Too Large";
  case 414: return "414 URI Too Long";
  case 415: return "415 Unsupported Media Type";
  case 416: return "416 Range Not Satisfiable";
  case 417: return "417 Expectation Failed";
  case 418: return "418 I'm a teapot";
  case 421: return "421 Misdirected Request";
  case 425: return "425 Too Early";
  case 426: return "426 Upgrade Required";
  case 428: return "428 Precondition Required";
  case 429: return "429 Too Many Requests";
  case 431: return "431 Request Header Fields Too Large";
  case 451: return "451 Unavailable For Legal Reasons";

  case 500: return "500 Internal Server Error";
  case 501: return "501 Not Implemented";
  case 502: return "502 Bad Gateway";
  case 503: return "503 Service Unavailable";
  case 504: return "504 Gateway Timeout";
  case 505: return "505 HTTP Version Not Supported";
  case 506: return "506 Variant Also Negotiates";
  case 510: return "510 Not Extended";
  case 511: return "511 Network Authentication Required";

  default: std::unreachable();
  } // clang-format on
}
std::string HTTP::method_to_string(HTTP::Method _)
{
  switch (_)
  { // clang-format off
  case HTTP::Method::GET:     return "GET";
  case HTTP::Method::HEAD:    return "HEAD";
  case HTTP::Method::POST:    return "POST";
  case HTTP::Method::PUT:     return "PUT";
  case HTTP::Method::DELETE:  return "DELETE";
  case HTTP::Method::CONNECT: return "CONNECT";
  case HTTP::Method::OPTIONS: return "OPTIONS";
  case HTTP::Method::TRACE:   return "TRACE";
  default: std::unreachable();
  } // clang-format on
}

std::map<std::string, std::string> HTTP::parse_cookies(std::string_view _)
{
  Iterator iterator(_);
  std::string name, value;
  std::map<std::string, std::string> map;

  while (iterator.next('='))
  {
    name = iterator.head;
    if (!iterator.next("; "))
    {
      value = iterator.tail;
      map[name] = value;
      break;
    }

    value = iterator.head;
    map[name] = value;
  }

  return map;
}

} // namespace protocol
