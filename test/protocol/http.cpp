#include "../../src/protocol/http/http.hpp"
#include "../../src/protocol/protocol.hpp"
#include "../test.hpp"

using namespace protocol;
using namespace test;

std::string parser_result_to_string(const ParserResult& l)
{
  switch (l)
  {
  case ParserResult::NeedMoreData:
    return "NeedMoreData";
  case ParserResult::Invalid:
    return "Invalid";
  case ParserResult::Complete:
    return "Complete";
  }
}

void assert_equal(const ParserResult& l, const ParserResult& r)
{
  if (l != r) throw ComparisonException(parser_result_to_string(l), parser_result_to_string(r));
}

void assert_equal(HTTP::Method& l, HTTP::Method r)
{
  if (l != r) throw ComparisonException(HTTP::method_to_string(l), HTTP::method_to_string(r));
}

void assert_equal(std::string_view l, std::string_view r)
{
  if (l != r) throw ComparisonException(l, r);
}

int protocol_http(int argc, char* argv[])
{
  int flag, _i, _j;
  bool _exception_caught;

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
    REQUIRE("path parsed correctly", { assert_equal(req.path, "/index.html"); })

    // this fails Host -> host
    REQUIRE("header parsed correctly", { assert_equal(req.headers["Host"], "example.com"); })
  }
  ENDSECTION;

  return flag;
}
