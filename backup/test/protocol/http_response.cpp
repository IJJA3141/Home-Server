#include "../../src/protocol/http/http.hpp"
#include "../../src/protocol/protocol.hpp"
#include "../test.hpp"
#include "util.hpp"

using namespace protocol;
using namespace test;

int protocol_http_response(int argc, char* argv[])
{
  int _flag = 0, _i = 0, _j = 0;
  bool _exception_caught = false;

  SECTION("Parse simple response")
  {
    std::string raw = "HTTP/1.1 200 OK\r\n"
                      "Content-Length: 0\r\n"
                      "\r\n";

    HTTP::Response::ParserContext ctx;
    ctx.reset();

    size_t consumed = HTTP::Response::parse(raw, ctx);

    REQUIRE("parser should complete", { assert_equal(ctx.result, ParserResult::Complete); })

    auto res = ctx.construct();

    REQUIRE("status parsed correctly", { assert_equal(res.status, 200); })

    REQUIRE("header parsed correctly", { assert_equal<std::string_view>(res.headers["content-length"], "0"); })
  }

  SECTION("Parse response with multiple headers")
  {
    std::string raw = "HTTP/1.1 404 Not Found\r\n"
                      "Content-Length: 0\r\n"
                      "Server: test\r\n"
                      "Connection: close\r\n"
                      "\r\n";

    HTTP::Response::ParserContext ctx;

    HTTP::Response::parse(raw, ctx);

    REQUIRE("parser should complete", { assert_equal(ctx.result, ParserResult::Complete); })

    auto res = ctx.construct();

    REQUIRE("all headers parsed", {
      assert_equal<size_t>(res.headers.size(), 3);
      assert_equal<std::string_view>(res.headers["content-length"], "0");
      assert_equal<std::string_view>(res.headers["server"], "test");
      assert_equal<std::string_view>(res.headers["connection"], "close");
    })
  }

  SECTION("Parse response with body")
  {
    std::string raw = "HTTP/1.1 200 OK\r\n"
                      "Content-Length: 5\r\n"
                      "\r\n"
                      "hello";

    HTTP::Response::ParserContext ctx;
    HTTP::Response::parse(raw, ctx);

    REQUIRE("parser should complete", { assert_equal(ctx.result, ParserResult::Complete); })

    auto res = ctx.construct();

    REQUIRE("status parsed", { assert_equal(res.status, 200); })

    REQUIRE("body parsed", { assert_equal<std::string_view>(res.body, "hello"); })
  }

  SECTION("Parser handles partial input")
  {
    HTTP::Response::ParserContext ctx;

    std::string part1 = "HTTP/1.1 200 OK\r\nContent-Len";
    std::string part2 = "gth: 5\r\n\r\nhello";

    ssize_t consumed = HTTP::Response::parse(part1, ctx);

    REQUIRE("parser should need more data", { assert_equal(ctx.result, ParserResult::NeedMoreData); })

    part1 = part1.substr(consumed);
    part1 += part2;

    HTTP::Response::parse(part1, ctx);

    REQUIRE("parser should complete after second chunk", { assert_equal(ctx.result, ParserResult::Complete); })
  }

  SECTION("Invalid status line detection")
  {
    std::string raw = "HTTP/1.1 ABC OK\r\n"
                      "\r\n";

    HTTP::Response::ParserContext ctx;

    HTTP::Response::parse(raw, ctx);

    REQUIRE("parser should detect invalid response", { assert_equal(ctx.result, ParserResult::Invalid); })
  }

  SECTION("Parser returns correct consumed byte count")
  {
    std::string raw = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\nEXTRA_DATA";

    HTTP::Response::ParserContext ctx;

    ssize_t consumed = HTTP::Response::parse(raw, ctx);

    REQUIRE("parser should complete", { assert_equal(ctx.result, ParserResult::Complete); })

    REQUIRE("parser should stop at end of response", {
      assert_equal<size_t>(consumed, std::string("HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n").size());
    })
  }

  SECTION("Response parsed correctly from streaming chunks")
  {
    HTTP::Response::ParserContext ctx;
    std::string raw = "";
    size_t ssize = 0;

    std::vector<std::string> chunks = {"HTTP/1.1 200", " OK\r\nContent-Le", "ngth: 5\r\n", "\r\nhello"};

    REQUIRE("parser should process chunk 1", {
      raw += chunks[0];
      ssize = HTTP::Response::parse(raw, ctx);
      raw = raw.substr(ssize);
      assert_equal(ctx.result, ParserResult::NeedMoreData);
    });

    REQUIRE("parser should process chunk 2", {
      raw += chunks[1];
      ssize = HTTP::Response::parse(raw, ctx);
      raw = raw.substr(ssize);
      assert_equal(ctx.result, ParserResult::NeedMoreData);
    });

    REQUIRE("parser should process chunk 3", {
      raw += chunks[2];
      ssize = HTTP::Response::parse(raw, ctx);
      raw = raw.substr(ssize);
      assert_equal(ctx.result, ParserResult::NeedMoreData);
    });

    REQUIRE("parser should complete after chunks", {
      raw += chunks[3];
      ssize = HTTP::Response::parse(raw, ctx);
      raw = raw.substr(ssize);
      assert_equal(ctx.result, ParserResult::Complete);
      assert(!raw.size());
    });
  }

  SECTION("Response serialization")
  {
    HTTP::Response res;
    res.version = HTTP::Version::HTTP_11;
    res.status = 200;
    res.headers["content-length"] = "5";
    res.body = "hello";

    std::string raw = (std::string)res;

    REQUIRE("serialized response contains status line", { assert(raw.starts_with("HTTP/1.1 200")); })

    REQUIRE("serialized response contains headers", { assert(raw.contains("content-length: 5")); })

    REQUIRE("serialized response contains body", { assert(raw.ends_with("hello")); })
  }

  ENDSECTION;

  return _flag;
}
