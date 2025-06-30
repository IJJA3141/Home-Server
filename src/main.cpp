#include "log.hpp"
#include "network/http.hpp"
#include "network/router.hpp"
#include "network/server.hpp"
#include <iostream>
#include <openssl/err.h>

const Response fallback{"HTTP/1.1",              // protocol
                        500,                     // code
                        "Internal Server Error", // message
                        {                        // headers
                         {"Content-Type", "text/html; charset=UTF-8"},
                         {"Content-Length", "162"},
                         {"Connection", "close"},
                         {"Date", "Sun, 29 Jun 2025 14:30:00 GMT"},
                         {"Server", "MyCustomServer/1.0"}},
                        // body
                        "<!DOCTYPE html>\n"
                        "<html>\n"
                        "<head><title>500 Internal Server Error</title></head>\n"
                        "<body>\n"
                        "<h1>500 Internal Server Error</h1>\n"
                        "<p>para</p>\n"
                        "</body>\n"
                        "</html>\n"};

int main(int argc, char* argv[])
{
  Router router(fallback);

  router.add(Method::GET, "/",
             [](Request _request) -> Response { return Response{"http/1.1", 200, "OK", {}, "Hello world!"}; });

  Tcp server(60, router);
  server.listen();

  std::string _;
  std::getline(std::cin, _);
  server.close();

  return 0;
}
