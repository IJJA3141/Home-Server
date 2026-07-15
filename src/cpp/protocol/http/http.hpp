#pragma once

#include "../protocol.hpp"
#include <map>
#include <optional>
#include <string>

namespace protocol
{
namespace http
{

enum Method
{
  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  TRACE
};
#define N_METHODS 8
static_assert(N_METHODS - 1 == (size_t)Method::TRACE);
std::optional<Method> parse_method(std::string_view);
constexpr std::string method_to_string(Method);

// path
std::map<std::string, std::string> parse_query(std::string_view);

enum struct Version
{
  HTTP_09,
  HTTP_10,
  HTTP_11,
  HTTP_20,
  HTTP_30
};
std::optional<Version> parse_version(std::string_view);
constexpr std::string version_to_string(Version);

using Header = std::pair<std::string, std::string>;
std::optional<Header> parse_header(std::string_view);
using Headers = std::unordered_map<Header::first_type, Header::second_type>;

std::map<std::string, std::string> parse_cookies(std::string_view);

using Status = int;
std::optional<Status> parse_status(std::string_view);
constexpr std::string status_to_string(Status);

struct Request
{
  Method method;
  std::string path;
  Version version;
  Headers header;

  size_t content_length;
  std::string host;
  std::string user_agent;

  class ParserContext;
};

class http::Request::ParserContext
{
public:
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
  } state_ = METHOD;

  Method method_;
  std::string path_;
  Version version_;
  Headers headers_;
  std::string body_;

  friend struct http::Request;
};

struct Response
{
};

} // namespace http
} // namespace protocol
