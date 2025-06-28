#include "../../src/log.hpp"
#include "../../src/network/http.hpp"

#include <map>
#include <string>
#include <vector>

int parsing_request(int argc, char* argv[])
{
  std::vector<std::string> requests{"GET /users/profile?user=john&id=42 HTTP/1.1\r\n"
                                    "Host: www.example.com\r\n"
                                    "User-Agent: MyBrowser/1.0\r\n"
                                    "Accept: */*\r\n"
                                    "\r\n",

                                    "POST /api/data/upload HTTP/1.1\r\n"
                                    "Host: api.example.com\r\n"
                                    "Content-Type: application/json\r\n"
                                    "Content-Length: 27\r\n"
                                    "\r\n"
                                    "{\"name\":\"John\",\"age\":30}",

                                    "GET /search/results?query=books&page=2#top HTTP/1.1\r\n"
                                    "Host: www.search.com\r\n"
                                    "Accept: text/html\r\n"
                                    "\r\n",

                                    "PUT /api/v1/users/123 HTTP/1.1\r\n"
                                    "Host: api.service.com\r\n"
                                    "Content-Type: application/json\r\n"
                                    "Content-Length: 31\r\n"
                                    "Authorization: Bearer token123\r\n"
                                    "\r\n"
                                    "{\"email\":\"new@example.com\"}",

                                    "DELETE /posts/456 HTTP/1.1\r\n"
                                    "Host: blog.example.com\r\n"
                                    "Authorization: Basic abc123\r\n"
                                    "\r\n"

  };

  std::vector<Method> expected_methods{Method::GET, Method::POST, Method::GET, Method::PUT, Method::DELETE};

  std::vector<std::vector<std::string>> expected_paths{{"users", "profile"},
                                                       {"api", "data", "upload"},
                                                       {"search", "results"},
                                                       {"api", "v1", "users", "123"},
                                                       {"posts", "456"}};

  std::vector<std::map<std::string, std::string>> expected_queries{{{"user", "john"}, {"id", "42"}},
                                                                   {},
                                                                   {{"query", "books"}, {"page", "2"}},
                                                                   {},
                                                                   {}};

  std::vector<std::string> expected_fragments{"", "", "top", "", ""};

  std::vector<std::string> expected_protocol{"HTTP/1.1", "HTTP/1.1", "HTTP/1.1", "HTTP/1.1", "HTTP/1.1"};

  std::vector<std::map<std::string, std::string>> expected_headers{
      {{"Host", "www.example.com"}, {"User-Agent", "MyBrowser/1.0"}, {"Accept", "*/*"}},
      {{"Host", "api.example.com"}, {"Content-Type", "application/json"}, {"Content-Length", "27"}},
      {{"Host", "www.search.com"}, {"Accept", "text/html"}},
      {{"Host", "api.service.com"}, {"Content-Type", "application/json"}, {"Content-Length", "31"}, {"Authorization", "Bearer token123"}},
      {{"Host", "blog.example.com"}, {"Authorization", "Basic abc123"}}
  };

  std::vector<std::string> expected_body{"", "{\"name\":\"John\",\"age\":30}", "", "{\"email\":\"new@example.com\"}", ""};

  int o = 0;

  for (int i = 0; i < requests.size(); ++i)
  {
    Request parsed_request;

    o += !check(Level::ERR, parse_request(requests[i]).state == Request::NONE, "couldn't parse", requests[i]);
    o += !check(Level::ERR, parsed_request.cmd.method == expected_methods[i], parsed_request.cmd.method, "!=", expected_methods[i]);
    o += !check(Level::ERR, parsed_request.cmd.url.path == expected_paths[i], parsed_request.cmd.url.path, "!=", expected_paths[i]);
    o += !check(Level::ERR, parsed_request.cmd.url.querys == expected_queries[i], parsed_request.cmd.url.querys, "!=", expected_queries[i]);
    o += !check(Level::ERR, parsed_request.cmd.url.fragment == expected_fragments[i], parsed_request.cmd.url.fragment, "!=", expected_fragments[i]);
    o += !check(Level::ERR, parsed_request.cmd.protocol == expected_protocol[i], parsed_request.cmd.protocol, "!=", expected_protocol[i]);
    o += !check(Level::ERR, parsed_request.headers == expected_headers[i], parsed_request.headers, "!=", expected_headers[i]);
    o += !check(Level::ERR, parsed_request.body == expected_body[i], parsed_request.body.size(), "!=", expected_body[i].size(), "\n\r", parsed_request.body, "!=", expected_body[i]);
  }

  Request parsed_request;
  o += !check(Level::ERR, parse_request("INVALID REQUEST").state != Request::NONE, "should have failed");

  check(Level::LOG, o, "parse_request test passed.");
  return o;
}
