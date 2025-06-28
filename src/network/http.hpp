#pragma once

#include "auth.hpp"

#include <map>
#include <string>
#include <string_view>
#include <vector>

constexpr int method_size = 8;
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
static_assert((method_size - 1) == Method::TRACE, "wrong size for method enum");

constexpr const auto method_to_string(const Method _method)
{
  switch (_method)
  {
  case GET: return "GET";
  case HEAD: return "HEAD";
  case POST: return "POST";
  case PUT: return "PUT";
  case DELETE: return "DELETE";
  case CONNECT: return "CONNECT";
  case OPTIONS: return "OPTIONS";
  case TRACE: return "TRACE";
  default: return "NAM";
  };
}

struct Url
{
  std::vector<std::string> path;
  std::map<std::string, std::string> querys;
  std::string fragment;
};

struct Command
{
  Method method;
  Url url;
  std::string protocol;
};

struct Request
{
  static const int error_size = 8;
  enum Error
  {
    READ,
    M_METHOD,
    M_URL,
    M_HEADER,
    I_METHOD,
    NOT_FOUND,
    CLOSED,
    NONE
  } state = Error::NONE;
  static_assert((Request::error_size - 1) == Error::NONE, "wrong size for request error enum");

  Command cmd;
  std::map<std::string, std::string> headers;

  std::string body;
};

struct Response
{
  std::string protocol;
  int code;
  std::string message;
  std::map<std::string, std::string> headers;

  std::string body;

  operator const std::string() const;
};

/**
 * TODO update
 * 
 * @brief Parses a string into a Request struct.
 *
 * This function analyzes the given input string and fills the provided
 * Request object with the corresponding parsed data.
 *
 * @param _model The input string to parse.
 * @param _request Reference to the Request object to populate.
 *
 * @return true if parsing fails, false otherwise.
 */
Request parse_request(const std::string_view _model);

/**
 * @brief Parses a string into a Method enum.
 *
 * Extracts and interprets the HTTP method (e.g., GET, POST) from the
 * given input string and assigns it to the Method enum.
 *
 * @param _model The input string containing the method only.
 * @param _method Reference to the Method object to populate.
 *
 * @return true if parsing fails, false otherwise.
 */
bool parse_method(const std::string_view _model, Method& _method);

/**
 * @brief Parses a string into a Url struct.
 *
 * Extracts the URL, Query(s) and Fragment from the given input string and assigns it to the Url struct.
 *
 * Format: /path?query#fragment
 *
 * @param _model The input string containing the URL.
 * @param _url Reference to the Url object to populate.
 *
 * @return true if parsing fails, false otherwise.
 */
bool parse_url(std::string_view _model, Url& _url);

/**
 * @brief Parses the query parameters from a URL string.
 * @brief Parses a string into query(s).
 *
 * Extracts key-value query parameters from the input string and stores
 * them in the provided map.
 *
 * @param _model The input string containing the query parameters.
 * @param _querys Reference to the map to store the parsed query parameters.
 *
 * @return true if parsing fails, false otherwise.
 */
bool parse_querys(std::string_view _model, std::map<std::string, std::string>& _querys);
