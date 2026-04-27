#pragma once

#include "../protocol.hpp"
#include <map>
#include <optional>
#include <string>

namespace protocol
{

struct HTTP
{
  enum struct Method
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

  enum struct Version
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
    std::string path;
    Version version;
    std::map<std::string, std::string> headers;
    std::string body;

    class ParserContext
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

    static ssize_t parse(std::span<const char>, ParserContext&);
    operator std::string() const;
  };

  struct Response
  {
    Version version;
    int status;
    std::map<std::string, std::string> headers;
    std::string body;

    class ParserContext
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
      std::map<std::string, std::string> headers_;
      std::string body_;

      friend struct HTTP::Response;
    };

    static ssize_t parse(std::span<const char>, ParserContext&);
    operator std::string() const;
  };

  static std::optional<Version> parse_version(std::string_view);
  static std::optional<int> parse_status(std::string_view);
  static std::optional<Method> parse_method(std::string_view);
  static std::optional<std::pair<std::string, std::string>> parse_header(std::string_view);

  static std::string version_to_string(Version);
  static std::string status_to_string(int);
  static std::string method_to_string(Method);
};

} // namespace protocol
