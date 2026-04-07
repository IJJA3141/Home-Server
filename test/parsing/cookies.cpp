#include "../../src/log.hpp"
#include "../../src/network/http.hpp"

#define TEST(_FN)                                                                                                 \
  if (_FN) return 1;

int equal(std::string_view _input, const std::map<std::string, std::string>& _expected)
{
  std::map<std::string, std::string> map;
  if (http::parse_cookies(_input, map))
  {
    err("parsing", _input, "failed.");
    return 1;
  }

  if (map == _expected) return 0;

  err("got", map, "but expected", _expected, ".");
  return 1;
}

int fails(std::string_view _input)
{
  std::map<std::string, std::string> map;
  if (http::parse_cookies(_input, map)) return 0;
  err("parsing", _input, "should have failed but didn't.");
  return 1;
}

int parsing_cookies(int argc, char* argv[])
{
  TEST(equal(
      "Cookie: session=0c6776f6-992a-4f0b-bfaf-6d14f4c2a4ff; sussion=5562398f-a750-43f1-bd89-e0e6aec4003e",
      {{"session", "0c6776f6-992a-4f0b-bfaf-6d14f4c2a4ff"}, {"sussion", "5562398f-a750-43f1-bd89-e0e6aec4003e"}}))

  TEST(equal("Cookie: session=0c6776f6-992a-4f0b-bfaf-6d14f4c2a4ff;",
             {{"session", "0c6776f6-992a-4f0b-bfaf-6d14f4c2a4ff"}}))

  TEST(fails("Cookie: "))
  TEST(fails("Coodae: "))
  TEST(fails("Co = 2"))

  return 0;
}
