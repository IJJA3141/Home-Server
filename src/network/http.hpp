#pragma once

#include "../log.hpp"

#include <map>
#include <string>
#include <string_view>

// 1xx Informational
#define HTTP_CONTINUE            100
#define HTTP_SWITCHING_PROTOCOLS 101
#define HTTP_EARLY_HINTS         103

// 2xx Success
#define HTTP_OK                            200
#define HTTP_CREATED                       201
#define HTTP_ACCEPTED                      202
#define HTTP_NON_AUTHORITATIVE_INFORMATION 203
#define HTTP_NO_CONTENT                    204
#define HTTP_RESET_CONTENT                 205
#define HTTP_PARTIAL_CONTENT               206
#define HTTP_IM_USED                       226

// 3xx - Redirection
#define HTTP_MULTIPLE_CHOICES   300
#define HTTP_MOVED_PERMANENTLY  301
#define HTTP_FOUND              302
#define HTTP_SEE_OTHER          303
#define HTTP_NOT_MODIFIED       304
#define HTTP_TEMPORARY_REDIRECT 307
#define HTTP_PERMANENT_REDIRECT 308

// 4xx - Client Errors
#define HTTP_BAD_REQUEST                     400
#define HTTP_UNAUTHORIZED                    401
#define HTTP_FORBIDDEN                       403
#define HTTP_NOT_FOUND                       404
#define HTTP_METHOD_NOT_ALLOWED              405
#define HTTP_NOT_ACCEPTABLE                  406
#define HTTP_PROXY_AUTHENTICATION_REQUIRED   407
#define HTTP_REQUEST_TIMEOUT                 408
#define HTTP_CONFLICT                        409
#define HTTP_GONE                            410
#define HTTP_LENGTH_REQUIRED                 411
#define HTTP_PRECONDITION_FAILED             412
#define HTTP_PAYLOAD_TOO_LARGE               413
#define HTTP_URI_TOO_LONG                    414
#define HTTP_UNSUPPORTED_MEDIA_TYPE          415
#define HTTP_RANGE_NOT_SATISFIABLE           416
#define HTTP_EXPECTATION_FAILED              417
#define HTTP_IM_A_TEAPOT                     418
#define HTTP_MISDIRECTED_REQUEST             421
#define HTTP_TOO_EARLY                       425
#define HTTP_UPGRADE_REQUIRED                426
#define HTTP_PRECONDITION_REQUIRED           428
#define HTTP_TOO_MANY_REQUESTS               429
#define HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE 431
#define HTTP_UNAVAILABLE_FOR_LEGAL_REASONS   451

// 5xx - Server Errors
#define HTTP_INTERNAL_SERVER_ERROR           500
#define HTTP_NOT_IMPLEMENTED                 501
#define HTTP_BAD_GATEWAY                     502
#define HTTP_SERVICE_UNAVAILABLE             503
#define HTTP_GATEWAY_TIMEOUT                 504
#define HTTP_HTTP_VERSION_NOT_SUPPORTED      505
#define HTTP_VARIANT_ALSO_NEGOTIATES         506
#define HTTP_NOT_EXTENDED                    510
#define HTTP_NETWORK_AUTHENTICATION_REQUIRED 511

// some MIME types
#define HTTP_MIME_NONE "none"

#define HTTP_MIME_PLAIN    "text/plain"
#define HTTP_MIME_HTML     "text/html"
#define HTTP_MIME_CSS      "text/css"
#define HTTP_MIME_MARKDOWN "text/markdown"

#define HTTP_MIME_JSON                  "application/json"
#define HTTP_MIME_JAVASCRIPT            "application/javascript"
#define HTTP_MIME_ZIP                   "application/zip"
#define HTTP_MIME_X_WWW_FORM_URLENCODED "application/x-www-form-urlencoded"
#define HTTP_MIME_OCTET_STREAM          "application/octet-stream"
#define HTTP_MIME_X_TAR                 "application/x-tar"

#define HTTP_MIME_JPEG    "image/jpeg"
#define HTTP_MIME_PNG     "image/png"
#define HTTP_MIME_GIF     "image/gif"
#define HTTP_MIME_WEBP    "image/webp"
#define HTTP_MIME_SVG_XML "image/svg+xml"
#define HTTP_MIME_BMP     "image/bmp"
#define HTTP_MIME_X_ICON  "image/x-icon"

namespace http
{

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

struct Url
{
  std::string path;
  std::map<std::string, std::string> querys;
  std::string fragment;
};

enum Protocol
{
  HTTP_09,
  HTTP_10,
  HTTP_11,
  HTTP_20,
  HTTP_30
};

struct Command
{
  Method method;
  Url url;
  Protocol protocol;
};

static const int error_size = 8;
enum Error
{
  READ,
  M_METHOD,
  M_URL,
  M_PROTOCOL,
  M_HEADER,
  I_METHOD,
  I_URL,
  NONE
};
static_assert((error_size - 1) == Error::NONE, "wrong size for request error enum");

struct Request
{
  Error state;
  Command cmd;
  std::map<std::string, std::string> headers;
  std::map<std::string, std::string> cookies;
  std::string body;

  operator const std::string() const;
};

struct Response
{
  Protocol protocol;
  int status;
  std::map<std::string, std::string> headers;
  std::string body;

  operator std::string();
};

/**
 *
 *
 * @brief
 *
 * @param _model
 *
 * @return
 */
Request parse_request(std::string_view _model);

/**
 *
 *
 * @brief
 *
 * @param _model
 * @param _method
 *
 * @return
 */
[[nodiscard]] bool parse_method(std::string_view _model, Method& _method);

/**
 * 
 * 
 * @brief 
 * 
 * @param _model
 * @param _url
 *
 * @return 
 */
[[nodiscard]] bool parse_url(std::string_view _model, Url& _url);

/**
 * 
 * 
 * @brief 
 * 
 * @param _model
 * @param _querys
 * @return 
 */
[[nodiscard]] bool parse_querys(std::string_view _model, std::map<std::string, std::string>& _querys);

/**
 * 
 * 
 * @brief 
 * 
 * @param _model
 * @param _protocol
 *
 * @return 0 = ok, 1 = failed
 */
[[nodiscard]] bool parse_protocol(std::string_view _model, Protocol& _protocol);

/**
 * 
 * 
 * @brief 
 * 
 * @param _model
 * @param _cookies
 *
 * @return 
 */
[[nodiscard]] bool parse_cookies(std::string_view _model, std::map<std::string, std::string>& _cookies);

constexpr const char* unparse_method(Method _protocol);
inline constexpr const char* unparse_status(int _status);
constexpr const char* unparse_protocol(Protocol _protocol);

}; // namespace http
