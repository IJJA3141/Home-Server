#pragma once

#include "../protocol.hpp"
#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace protocol
{

struct HTTP
{
  // clang-format off
  using header = std::pair<std::string, std::string>;
  using status = int;

  enum Method { GET, HEAD, POST, PUT, DELETE, CONNECT, OPTIONS, TRACE };
  static std::optional<Method> parse_method(std::string_view);
  static std::string method_to_string(Method);
  #define N_METHODS 8
  static_assert(N_METHODS - 1 == (size_t)Method::TRACE);

  enum struct Version { HTTP_09, HTTP_10, HTTP_11, HTTP_20, HTTP_30 };
  static std::optional<Version> parse_version(std::string_view);
  static std::string version_to_string(Version);

  static std::optional<header> parse_header(std::string_view);
  static std::map<std::string, std::string> parse_cookies(std::string_view);
  static std::optional<status> parse_status(std::string_view);
  static std::string status_to_string(status);
  // clang-format on

  struct Request
  {
    Method method;
    std::string path;
    Version version;
    std::map<std::string, std::string> headers;
    std::string body;

    class ParserContext;
    static size_t parse(std::span<const char>, ParserContext&);
    operator std::string() const;
  };

  struct Response
  {
    Version version;
    int status;
    std::string message;
    std::map<std::string, std::string> headers;
    std::string body;

    class ParserContext;
    static size_t parse(std::span<const char>, ParserContext&);
    operator std::string() const;
  };

  static Response standard_response(int status);
};

class HTTP::Request::ParserContext
{
public:
  std::string error_msg;
  ParserResult result;
  Request construct();

  void reset(); // ???
  ParserContext();

private:
  enum
  {
    METHOD,
    PATH,
    VERSION,
    HEADERS,
    BODY,
  } state_;

  Method method_;
  std::string path_;
  Version version_;
  std::map<std::string, std::string> headers_;
  std::string body_;

  friend struct HTTP::Request;
};

class HTTP::Response::ParserContext
{
public:
  std::string error_msg;
  ParserResult result;
  Response construct();

  void reset(); // ???
  ParserContext();

private:
  enum
  {
    VERSION,
    STATUS,
    STATUS_NAME,
    HEADERS,
    BODY,
  } state_;

  Version version_;
  int status_;
  std::string message_;
  std::map<std::string, std::string> headers_;
  std::string body_;

  friend struct HTTP::Response;
};

} // namespace protocol
