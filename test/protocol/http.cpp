#include "../../src/protocol/http.hpp"
#include "../test.hpp"
#include <string_view>

using namespace protocol;
using namespace test;

using mss = std::map<std::string, std::string>;

// Assuming these helpers exist or are implemented in your framework
constexpr HTTP::Request Request(HTTP::Method, const std::string&, HTTP::Version, const mss&, const std::string&);
constexpr HTTP::Response Response(HTTP::Version, int, const mss& headers, const std::string& body);
constexpr void assert_equal(const HTTP::Response& l, const HTTP::Response& r);
constexpr void assert_equal(const HTTP::Request& l, const HTTP::Request& r);

int flag = 0;

int protocol_http(int argc, char* argv[])
{
  int _i = 0, _j = 0;
  bool _exception_caught;

  SECTION("request-line syntax")
  {
    REQUIRE("valid request-line", {
      const auto raw = "GET /index.html HTTP/1.1\r\n\r\n";
      const auto expected = Request(HTTP::GET, "/index.html", HTTP::HTTP_11, {}, "");
      assert_equal(HTTP::Request(raw), expected);
    });

    REQUIRE_THROW("missing space separators", HTTP::ParsingException, {
      const auto raw = "GET/index.html HTTP/1.1\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });

    REQUIRE_THROW("extra tokens in request line", HTTP::ParsingException, {
      const auto raw = "GET / HTTP/1.1 EXTRA\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });

    REQUIRE_THROW("request-line without CRLF", HTTP::ParsingException, {
      const auto raw = "GET / HTTP/1.1";
      const auto _ = HTTP::Request(raw);
    });

    REQUIRE("valid methods", {
      const auto raw = "PUT /resource HTTP/1.1\r\n\r\n";
      const auto expected = Request(HTTP::PUT, "/resource", HTTP::HTTP_11, {}, "");
      assert_equal(HTTP::Request(raw), expected);
    });

    REQUIRE_THROW("method must be case-sensitive", HTTP::ParsingException, {
      const auto raw = "get / HTTP/1.1\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });

    REQUIRE_THROW("invalid method token", HTTP::ParsingException, {
      const auto raw = "GE@T / HTTP/1.1\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });

    REQUIRE("valid version", {
      const auto raw = "GET / HTTP/1.0\r\n\r\n";
      const auto expected = Request(HTTP::GET, "/", HTTP::HTTP_10, {}, "");
      assert_equal(HTTP::Request(raw), expected);
    });

    REQUIRE_THROW("invalid version prefix", HTTP::ParsingException, {
      const auto raw = "GET / HTTX/1.1\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });

    REQUIRE_THROW("invalid version digits", HTTP::ParsingException, {
      const auto raw = "GET / HTTP/x.y\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });
  }
  ENDSECTION;

  SECTION("header-field syntax")
  {
    REQUIRE("valid header", {
      const auto raw = "GET / HTTP/1.1\r\nHost: example.com\r\n\r\n";
      const auto expected = Request(HTTP::GET, "/", HTTP::HTTP_11, {{"host", "example.com"}}, "");
      assert_equal(HTTP::Request(raw), expected);
    });

    REQUIRE_THROW("header missing colon", HTTP::ParsingException, {
      const auto raw = "GET / HTTP/1.1\r\nHost example.com\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });

    REQUIRE_THROW("invalid header name", HTTP::ParsingException, {
      const auto raw = "GET / HTTP/1.1\r\nBad Header: value\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });
  }
  ENDSECTION;

  SECTION("whitespace rules")
  {
    REQUIRE("valid with whitespace", {
      const auto raw = "GET / HTTP/1.1\r\nHost:    example.com\r\n\r\n";
      const auto expected = Request(HTTP::GET, "/", HTTP::HTTP_11, {{"host", "example.com"}}, "");
      assert_equal(HTTP::Request(raw), expected);
    });

    REQUIRE_THROW("header cannot start with whitespace", HTTP::ParsingException, {
      const auto raw = "GET / HTTP/1.1\r\n Host: example.com\r\n\r\n";
      const auto _ = HTTP::Request(raw);
    });
  }
  ENDSECTION;

  SECTION("message framing")
  {
    REQUIRE("valid body", {
      const auto raw = "POST / HTTP/1.1\r\n\r\nBODY";
      const auto expected = Request(HTTP::POST, "/", HTTP::HTTP_11, {}, "BODY");
      assert_equal(HTTP::Request(raw), expected);
    });
  }
  ENDSECTION;

  SECTION("response")
  {
    REQUIRE("valid response", {
      const auto raw = "HTTP/1.1 404 Not Found\r\n\r\n";
      const auto expected = Response(HTTP::HTTP_11, 404, {}, "");
      assert_equal(HTTP::Response(raw), expected);
    });

    REQUIRE_THROW("status code must be 3 digits", HTTP::ParsingException, {
      const auto raw = "HTTP/1.1 20 OK\r\n\r\n";
      const auto _ = HTTP::Response(raw);
    });

    REQUIRE_THROW("missing reason phrase", HTTP::ParsingException, {
      const auto raw = "HTTP/1.1 200\r\n\r\n";
      const auto _ = HTTP::Response(raw);
    });
  }
  ENDSECTION;

  return flag;
}

constexpr protocol::HTTP::Request Request(HTTP::Method method, const std::string& url, HTTP::Version version,
                                          const std::map<std::string, std::string>& headers,
                                          const std::string& body)
{
  HTTP::Request req;
  req.method = method;
  req.url = url;
  req.version = version;
  req.headers = headers;
  req.body = body;
  return req;
}

constexpr protocol::HTTP::Response Response(HTTP::Version version, int status,
                                            const std::map<std::string, std::string>& headers,
                                            const std::string& body)
{
  protocol::HTTP::Response res;
  res.version = version;
  res.status = status;
  res.headers = headers;
  res.body = body;
  return res;
}

constexpr void assert_equal(const HTTP::Request& l, const HTTP::Request& r)
{
  if (l.method != r.method)
    throw ComparisonException(HTTP::serialize_method(l.method), HTTP::serialize_method(r.method));
  if (l.url != r.url) throw ComparisonException(l.url, r.url);
  if (l.version != r.version)
    throw ComparisonException(HTTP::serialize_version(l.version), HTTP::serialize_version(r.version));
  if (l.headers != r.headers) throw ComparisonException(l.headers, r.headers);
  if (l.body != r.body) throw ComparisonException(l.body, r.body);
}

constexpr void assert_equal(const HTTP::Response& l, const HTTP::Response& r)
{
  if (l.version != r.version)
    throw ComparisonException(HTTP::serialize_version(l.version), HTTP::serialize_version(r.version));
  if (l.status != r.status) throw ComparisonException(l.status, r.status);
  if (l.headers != r.headers) throw ComparisonException(l.headers, r.headers);
  if (l.body != r.body) throw ComparisonException(l.body, r.body);
}
