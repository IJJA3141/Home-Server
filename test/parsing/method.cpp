#include "../../src/log.hpp"
#include "../../src/network/http.hpp"

#define TEST(_FN)                                                                                                 \
  if (_FN) return 1;

int parsing_method(int argc, char* argv[])
{
  http::Method parsed_method;
  for (int i = http::Method::GET; i <= http::Method::TRACE; ++i)
  {
    TEST(check(Level::ERR, parsed_method == (http::Method)i, parsed_method, " != ", i));
    TEST(check(Level::ERR, !parse_method(http::unparse_method((http::Method)i), parsed_method),
               http::unparse_method((http::Method)i), "failed", i, "not equal to", parsed_method));
  }

  TEST(!check(Level::ERR, parse_method("fjksdlaf", parsed_method), "should have failed"));
  return 0;
}
