#pragma once

#include "protocol.hpp"
#include <exception>
#include <format>
#include <map>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <utility>

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

namespace protocol
{

struct HTTP
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

  enum Version
  {
    HTTP_09,
    HTTP_10,
    HTTP_11,
    HTTP_20,
    HTTP_30
  };

  struct Request
  {
    Method method;
    std::string url;
    Version version;
    std::map<std::string, std::string> headers;
    std::string body;

    Request() = default;
    Request(const std::string_view _s);
    operator std::string() const;
  };

  struct Response
  {
    Version version;
    int status;
    std::map<std::string, std::string> headers;
    std::string body;

    Response() = default;
    Response(const std::string_view _s);
    operator std::string() const;
  };

  struct ShortException : std::exception // TODO change name and better exception
  {
    const char* what() const noexcept override { return ""; }
  };

  struct ParsingException : std::exception
  {
    const std::string reason;

    template <typename... Args>
    ParsingException(std::format_string<Args...> _fmt, Args&&... _args)
        : reason{std::format(_fmt, std::forward<Args>(_args)...)} {};

    const char* what() const noexcept override { return this->reason.c_str(); }
  };

  static int deserialize_status(const std::string_view);
  static HTTP::Method deserialize_method(const std::string_view);
  static HTTP::Version deserialize_version(const std::string_view);
  static std::pair<std::string, std::string> deserialize_header(const std::string_view);

  static std::string serialize_method(HTTP::Method);
  static std::string serialize_version(HTTP::Version);
  static std::string serialize_status(int);

  template <Serializable S> static ssize_t send(const int socket, const S& s, const int flag = 0)
  {
    const std::string str(s);
    return ::send(socket, str.c_str(), s.size(), flag);
  }

  template <Deserializable D> static ssize_t recv(const int socket, D& d, const int flag = 0)
  {
    // TODO

    // auto log = Logger::New();
    //
    // char buf[BUFFER_SIZE + 1]; // plus one for '\0'?
    // std::string s;
    // ssize_t bt;
    //
    // bt = ::recv(socket, buf, BUFFER_SIZE, flag);
    // log.info("received bt = {}", bt);
    //
    // if (bt <= 0) return bt; // client closed connection or recv failed
    //
    // while (bt == BUFFER_SIZE)
    // {
    //   s += buf;
    //   bt = ::recv(socket, buf, BUFFER_SIZE, flag);
    //   if (bt < 0) return bt; // recv failed
    // }
    //
    // buf[bt] = '\0';
    // s += buf;
    // d = D(s);
    //
    // return bt;
  }
};

} // namespace protocol
