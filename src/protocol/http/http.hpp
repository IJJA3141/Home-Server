#include "../protocol.hpp"
#include <map>
#include <string>

namespace protocol
{

struct http
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
    Version version;
    std::string path;
    Method method;
    std::map<std::string, std::string> headers;
    std::string body;

    class ParserContext
    {
    public:
      std::string error_msg;
      ParserResult result;
      Request construct();

    private:
      enum
      {
        COMMAND,
        HEADER,
        BODY
      } state_;

      Version version_;
      std::string path_;
      Method method_;
      std::map<std::string, std::string> headers_;
      std::string body_;
    };

    static ssize_t parse(std::span<const std::byte>, ParserContext&);
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

    private:
      enum
      {
        COMMAND,
        HEADER,
        BODY
      } state_;

      Version version_;
      int status_;
      std::map<std::string, std::string> headers_;
      std::string body_;
    };

    static ssize_t parse(std::span<const std::byte>, ParserContext&);
    operator std::string() const;
  };

  Version parse_version(std::string_view);
  int parse_status(std::string_view);
  Method parse_method(std::string_view);
  std::pair<std::string, std::string> parse_header(std::string_view);

  std::string version_to_string(Version);
  std::string status_to_string(int);
  std::string method_to_string(Method);
  std::string headers_to_string(const std::map<std::string, std::string>&);
};

} // namespace protocol
