#include "network/http.hpp"
#include "network/router.hpp"
#include "network/server.hpp"
#include <iostream>
#include <openssl/err.h>

int main(int argc, char* argv[])
{
  Router router;
  Tcp server(5);

  router.add(Method::GET, "/",
             [](Request _request) -> Response { return Response{"http/1.1", 200, "OK", {}, "Hello world!"}; });

  server.bind(60);
  server.listen();

  std::string _;
  std::cin >> _;
  server.close();

  return 0;
}
