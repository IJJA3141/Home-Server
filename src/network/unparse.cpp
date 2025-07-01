#include "http.hpp"

#include <sstream>
#include <string>
#include <utility>

http::Request::operator const std::string() const
{
  std::stringstream ss;
  ss << unparse_method(this->cmd.method) << " " << this->cmd.url.path;
  ss << " " << unparse_protocol(this->cmd.protocol) << "\r\n";

  if (!this->cmd.url.querys.empty())
  {
    auto it = this->cmd.url.querys.begin();

    ss << "?";

    while (it != --this->cmd.url.querys.end())
      ss << it->first << "=" << it++->second << "&";

    ss << it->first << "=" << it->second << "&";
  }

  if (!this->cmd.url.fragment.empty()) ss << "#" << this->cmd.url.fragment;

  for (const auto& header : this->headers)
    ss << header.first << ": " << header.second << "\r\n";

  ss << "\r\n" << this->body;
  return ss.str();
}

http::Response::operator std::string() {
  std::stringstream ss;

  this->headers["Content-Length"] = std::to_string(this->body.size());
  this->headers["Content-Type"] = this->type;

  ss << unparse_protocol(this->protocol) << " ";
  ss << unparse_status(this->status) << "\r\n";

  for (const auto& header : this->headers)
    ss << header.first << ": " << header.second << "\r\n";

  // ss << "\r\n" << this->body << "\r\n";
  ss << "\r\n" << this->body;
  return ss.str();
}

constexpr const char* http::unparse_method(http::Method _method)
{
  switch (_method)
  {
  case http::GET: return "GET";
  case http::HEAD: return "HEAD";
  case http::POST: return "POST";
  case http::PUT: return "PUT";
  case http::DELETE: return "DELETE";
  case http::CONNECT: return "CONNECT";
  case http::OPTIONS: return "OPTIONS";
  case http::TRACE: return "TRACE";

  default: std::unreachable();
  };
}

constexpr const char* http::unparse_protocol(Protocol _protocol)
{
  switch (_protocol)
  {
  case HTTP_09: return "HTTP/0.9";
  case HTTP_10: return "HTTP/1.0";
  case HTTP_11: return "HTTP/1.1";
  case HTTP_20: return "HTTP/2.0";
  case HTTP_30: return "HTTP/3.0";

  default: std::unreachable();
  }
}

constexpr const char* http::unparse_status(int _code)
{
  switch (_code)
  {
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
  }
}
