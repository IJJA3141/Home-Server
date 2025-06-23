#include "../../src/log.hpp"
#include "../../src/network/http.hpp"

#include <vector>

int parsing_querys(int argc, char* argv[])
{
  const std::vector<std::string> str = {"name=JohnDoe&age=30&city=NewYork",
                                        "id=12345",
                                        "search=chatgpt&lang=en&sort=asc",
                                        "q=openai&limit=10&page=2",
                                        "user=alice&token=abc123xyz",
                                        "category=books&author=tolkien&year=1954",
                                        "a=1&b=2&c=3&d=4",
                                        "session=xyz789",
                                        "filter=active&order=desc",
                                        "debug=true&verbose=false"};

  const std::vector<std::map<std::string, std::string>> kv = {
      {{"name", "JohnDoe"}, {"age", "30"}, {"city", "NewYork"}},
      {{"id", "12345"}},
      {{"search", "chatgpt"}, {"lang", "en"}, {"sort", "asc"}},
      {{"q", "openai"}, {"limit", "10"}, {"page", "2"}},
      {{"user", "alice"}, {"token", "abc123xyz"}},
      {{"category", "books"}, {"author", "tolkien"}, {"year", "1954"}},
      {{"a", "1"}, {"b", "2"}, {"c", "3"}, {"d", "4"}},
      {{"session", "xyz789"}},
      {{"filter", "active"}, {"order", "desc"}},
      {{"debug", "true"}, {"verbose", "false"}}};

  std::map<std::string, std::string> map;
  int o = 0;

  for (int i = 0; i < kv.size(); ++i)
  {
    o += !check(Level::ERR, parse_querys(str[i], map), "couldn't parse", str[i]);
    o += !check(Level::ERR, map == kv[i], map, "should be equal to", kv[i]);
    map.clear();
  }

  o += !check(Level::ERR, parse_querys("a2&c=3&d=4", map), "should have failed");

  return o;
}
