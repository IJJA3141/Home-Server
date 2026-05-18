#include "../../src/protocol/http/http.hpp"
#include "../../src/protocol/protocol.hpp"
#include "../test.hpp"
#include "util.hpp"

using namespace protocol;
using namespace test;

int protocol_http_request(int argc, char* argv[])
{
  int _flag = 0, _i = 0, _j = 0;
  bool _exception_caught = false;

  SECTION("Parse simple GET request")
  {
    std::string raw = "GET /index.html HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "\r\n";

    HTTP::Request::ParserContext ctx;
    ctx.reset();

    ssize_t consumed = HTTP::Request::parse(raw, ctx);

    REQUIRE("parser should complete", { assert_equal(ctx.result, protocol::ParserResult::Complete); })

    auto req = ctx.construct();

    REQUIRE("method parsed correctly", { assert_equal(req.method, HTTP::Method::GET); })
    REQUIRE("path parsed correctly", { assert_equal<std::string_view>(req.path, "/index.html"); })
    REQUIRE("header parsed correctly", { assert_equal<std::string_view>(req.headers["host"], "example.com"); })
  }

  SECTION("Parse request with multiple headers")
  {
    std::string raw = "GET /api HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "User-Agent: test-client\r\n"
                      "Accept: */*\r\n"
                      "\r\n";

    HTTP::Request::ParserContext ctx;

    HTTP::Request::parse(raw, ctx);
    auto req = ctx.construct();

    REQUIRE("all headers parsed", {
      assert_equal<size_t>(req.headers.size(), 3);
      assert_equal<std::string_view>(req.headers["host"], "example.com");
      assert_equal<std::string_view>(req.headers["user-agent"], "test-client");
      assert_equal<std::string_view>(req.headers["accept"], "*/*");
    })
  }

  SECTION("Parse POST request with body")
  {
    std::string raw = "POST /submit HTTP/1.1\r\n"
                      "Host: example.com\r\n"
                      "Content-Length: 5\r\n"
                      "\r\n"
                      "hello";

    HTTP::Request::ParserContext ctx;
    HTTP::Request::parse(raw, ctx);

    REQUIRE("parser should complete", { assert_equal(ctx.result, protocol::ParserResult::Complete); })

    auto req = ctx.construct();

    REQUIRE("method parsed", { assert_equal(req.method, HTTP::Method::POST); })
    REQUIRE("body parsed", { assert_equal<std::string_view>(req.body, "hello"); })
  }

  SECTION("Parser handles partial input")
  {
    HTTP::Request::ParserContext ctx;

    std::string part1 = "GET / HTTP/1.1\r\nHost:";
    std::string part2 = " example.com\r\n\r\n";

    ssize_t consumed = HTTP::Request::parse(part1, ctx);
    REQUIRE("parser should need more data", { assert_equal(ctx.result, protocol::ParserResult::NeedMoreData); })
    REQUIRE("parser should have consumed right amount", { assert_equal<size_t>(consumed, sizeof("GET / HTTP/1.1\r\n") - 1); })

    part1 = part1.substr(consumed); // remove consumed bytes
    part1 += part2;                 // more received
    consumed = HTTP::Request::parse(part1, ctx);
    REQUIRE("parser should complete after second chunk",
            { assert_equal(ctx.result, protocol::ParserResult::Complete); })
  }

  SECTION("Invalid method detection")
  {
    std::string raw = "BADMETHOD / HTTP/1.1\r\n"
                      "\r\n";

    HTTP::Request::ParserContext ctx;
    HTTP::Request::parse(raw, ctx);
    REQUIRE("parser should detect invalid request", { assert_equal(ctx.result, protocol::ParserResult::Invalid); })
  }

  // not a fan of this
  // SECTION("Invalid header termination")
  // {
  //   std::string raw = "GET / HTTP/1.1\n"
  //                     "Host: example.com\n"
  //                     "\n";
  //
  //   HTTP::Request::ParserContext ctx;
  //
  //   HTTP::Request::parse(raw, ctx);
  //
  //   REQUIRE("parser should reject malformed headers",
  //           { assert_equal(ctx.result, protocol::ParserResult::Invalid); })
  // }

  SECTION("Parser returns correct consumed byte count")
  {
    std::string raw = "GET / HTTP/1.1\r\n\r\nEXTRA_DATA";

    HTTP::Request::ParserContext ctx;

    ssize_t consumed = HTTP::Request::parse(raw, ctx);

    REQUIRE("parser should complete", { assert_equal(ctx.result, protocol::ParserResult::Complete); })

    REQUIRE("parser should stop at end of request",
            { assert_equal<size_t>(consumed, std::string("GET / HTTP/1.1\r\n\r\n").size()); })
  }

  SECTION("Request parsed correctly from streaming chunks")
  {
    HTTP::Request::ParserContext ctx;

    std::vector<std::string> chunks = {"GET /ind", "ex.html HTTP/1.1\r\nHo", "st: example.com\r\n", "\r\n"};

    for (auto& chunk : chunks)
    {
      HTTP::Request::parse(chunk, ctx);

      if (ctx.result == protocol::ParserResult::Complete) break;
    }

    REQUIRE("parser should complete after chunks", { assert_equal(ctx.result, protocol::ParserResult::Complete); })
  }
  ENDSECTION;

  return _flag;
}
