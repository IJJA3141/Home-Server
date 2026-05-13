#include "../../src/routing/routing.hpp"
#include "../../src/utils/exception.hpp"
#include "../test.hpp"
#include <format>
#include <string>
#include <string_view>

using namespace test;

int routing_route(int argc, char* argv[])
{
  int _flag = 0, _i = 0, _j = 0;
  bool _exception_caught = false;

  SECTION("Invalid characters in route")
  {
    for (const char& c : std::string(":?#@!$&'()+,;=<>\"{}|\\^` %"))
    {
      REQUIRE_THROW(std::format("reject invalid char {}", c), malformed_route,
                    { Route rt(std::format("/a/{}b", c)); });
    }
  }

  SECTION("Route must start with slash")
  {
    REQUIRE("missing leading slash", { assert_equal<std::string_view>(Route("/"), ""); });
    REQUIRE_THROW("missing leading slash", malformed_route, { Route r("users"); });
  }

  SECTION("Valid simple route")
  {
    REQUIRE("accept basic route", { assert_equal<std::string_view>(Route("/users"), "users"); });
    REQUIRE("accept nested route", { assert_equal<std::string_view>(Route("/users/list"), "users/list"); });
  }

  SECTION("Wildcard rules")
  {
    REQUIRE("valid wildcard route", { assert_equal<std::string_view>(Route("/assets/*"), "assets/*"); });
    REQUIRE_THROW("wildcard not at end", malformed_route, { Route r("/users/*/list"); });
    REQUIRE_THROW("multiple wildcards", malformed_route, { Route r("/users/*/files/*"); });
  }

  SECTION("Bracket validation")
  {
    REQUIRE("valid param route",
            { assert_equal<std::string_view>(Route("/users/[id]/path"), "users/[id]/path"); });
    REQUIRE("unnamed leaf with parameter",
            { assert_equal<std::string_view>(Route("/users/[id]/"), "users/[id]/"); });
    REQUIRE_THROW("last segment cannot be parametric", malformed_route,
                  { assert_equal<std::string_view>(Route("/users/[id]"), "users/[id]"); });
    REQUIRE_THROW("unclosed bracket", malformed_route, { Route r("/users/[id"); });
    REQUIRE_THROW("nested brackets", malformed_route, { Route r("/users/[[id]]"); });
    REQUIRE_THROW("unopened closing bracket", malformed_route, { Route r("/users/id]"); });
  }

  SECTION("Trailing slash normalization")
  {
    REQUIRE("trailing slash removed", { assert_equal<std::string_view>(Route("/users/"), "users"); };)
    REQUIRE("root path valid", { assert_equal<std::string_view>(Route("/"), ""); });
  }

  SECTION("Complex valid routes")
  {
    REQUIRE_THROW("mixed static + param", malformed_route, { Route("/api/v1/users/[id]"); });
    REQUIRE("mixed static + param",
            { assert_equal<std::string_view>(Route("/api/v1/users/[id]/"), "api/v1/users/[id]/"); });
    REQUIRE("mixed with wildcard", { assert_equal<std::string_view>(Route("/assets/v1/*"), "assets/v1/*"); });
  }

  ENDSECTION;

  return _flag;
}
