#include "../server/router.hpp"
#include "test.hpp"

void test::router()
{
  Router router;

  bool failed1 = true;
  bool failed2 = true;

  router.add(Method::GET, "/over/there", [&](Request _req) -> Response {
    failed1 = false;
    return {};
  });

  router.add(Method::GET, "/over/[user]/there", [&](Request _req) -> Response {
    failed2 = _req.url_args["user"] != "IJJA";
    return {};
  });

  router.respond(Request("GET /over/there prot\r\nHeader: 0\r\n\r\nbody", Client::Type::SSL));
  router.respond(Request("GET /over/IJJA/there prot\r\nHeader: 0\r\n\r\nbody", Client::Type::SSL));

  if (failed1 || failed2) {
    ERR("router test failed");
    return;
  }

  LOG("router test passed");
  return;
}
