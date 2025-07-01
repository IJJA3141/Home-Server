#include "../../src/log.hpp"
#include "../../src/network/http.hpp"

#include <vector>

int parsing_url(int argc, char* argv[])
{
  std::vector<std::string> urls{"/example",
                                "/example/page",
                                "/example/page?query=123",
                                "/example/page?query=123&sort=asc",
                                "/example/page#section1",
                                "/example/page?query=123#section1",
                                "/test",
                                "/test/resource?id=456",
                                "/test/resource#footer",
                                "/test/resource?id=456#footer"};

  std::vector<std::string> fdafd{"/example",       "/example/page", "/example/page", "/example/page", "/example/page",  "/example/page", "/test",         "/test/resource", "/test/resource", "/test/resource"};

  std::vector<std::map<std::string, std::string>> fjkdla{{},
                                                         {},
                                                         {{"query", "123"}},
                                                         {{"query", "123"}, {"sort", "asc"}},
                                                         {},
                                                         {{"query", "123"}},
                                                         {},
                                                         {{"id", "456"}},
                                                         {},
                                                         {{"id", "456"}}};

  std::vector<std::string> frag = {"", "", "", "", "section1", "section1", "", "", "footer", "footer"};

  int o = 0;

  for (int i = 0; i < urls.size(); ++i)
  {
    http::Url parsed_url;
    o += !check(Level::ERR, !parse_url(urls[i], parsed_url), "couldn't parse", urls[i]);
    o += !check(Level::ERR, parsed_url.path == fdafd[i], parsed_url.path, "!=", fdafd[i]);
    o += !check(Level::ERR, parsed_url.querys == fjkdla[i], parsed_url.querys, "!=", fjkdla[i]);
    o += !check(Level::ERR, parsed_url.fragment == frag[i], parsed_url.fragment, "!=", frag[i]);
  }

  http::Url parsed_url;
  o += !check(Level::ERR, parse_url("jflds;a", parsed_url), "should have failed");

  check(Level::LOG, o, "parsing_url test passed.");
  return o;
}
