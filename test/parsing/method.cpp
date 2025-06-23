#include "../../src/log.hpp"
#include "../../src/network/http.hpp"

int parsing_method(int argc, char* argv[])
{

  Method parsed_method;
  int o = 0;

  for (int i = Method::GET; i <= Method::TRACE; ++i)
  {
    o += !check(Level::ERR, !parse_method(method_to_string((Method)i), parsed_method), method_to_string((Method)i), "failed", i, "not equal to", parsed_method);
    o += !check(Level::ERR, parsed_method == (Method)i, parsed_method, " != ", i);
  }

  o += !check(Level::ERR, parse_method("fjksdlaf", parsed_method), "should have failed");

  return o;
}
