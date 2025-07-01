#include "../../src/log.hpp"
#include "../../src/network/http.hpp"

int parsing_method(int argc, char* argv[])
{

  http::Method parsed_method;
  int o = 0;

  for (int i = http::Method::GET; i <= http::Method::TRACE; ++i)
  {
    o += !check(Level::ERR, !parse_method(http::unparse_method((http::Method)i), parsed_method), http::unparse_method((http::Method)i), "failed", i, "not equal to", parsed_method);
    o += !check(Level::ERR, parsed_method == (http::Method)i, parsed_method, " != ", i);
  }

  o += !check(Level::ERR, parse_method("fjksdlaf", parsed_method), "should have failed");

  check(Level::LOG, o, "parsing_method test passed.");
  return o;
}
