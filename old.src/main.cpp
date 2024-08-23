#include "log.hpp"
#include "server/router.hpp"
#include "server/server.hpp"
#include "test.hpp"

#include <string>

int main(int _argc, char *_argv[])
{
  Router router(_argv[0]);

  Tcp http(&router);

  router.add(Method::GET, "/", [](Request _) -> std::string {
    LOG(_.body);
    return "";
  });

  http.bind(80);
  http.listen();

  return 0;
}
