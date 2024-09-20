#include "../server/router.hpp"
#include "test.hpp"

bool failed1 = true;

Response teststatic(Request _)
{
  failed1 = false;
  return {};
}

void test::router()
{
  Router router;

  bool failed2 = true;
  bool failed3 = false;

  router.add(Method::GET, "/over/there", teststatic);

  router.add(Method::GET, "/over/[user]/there", [&](Request _req) -> Response {
    failed2 = _req.url_args["user"] != "IJJA";
    return {};
  });

  router.respond(Request("GET /over/there prot\r\nHeader: 0\r\n\r\nbody", Client::Type::SSL));
  router.respond(Request("GET /over/IJJA/there prot\r\nHeader: 0\r\n\r\nbody", Client::Type::SSL));

  std::function<Response(Request)> a = [](Request _req) -> Response { return {{"", 0}, {}, ""}; };
  router.add_error_handler(Request::Failure::METHOD, a);

  router.add_error_handler(Request::Failure::TRAILING,
                           [](Request _req) -> Response { return {{"", 1}, {}, ""}; });

  router.add_error_handler(Request::Failure::SIZE,
                           [](Request _req) -> Response { return {{"", 2}, {}, ""}; });

  router.add_error_handler(Request::Failure::PATH,
                           [](Request _req) -> Response { return {{"", 3}, {}, ""}; });

  router.add_error_handler(Request::Failure::LENGTH,
                           [](Request _req) -> Response { return {{"", 4}, {}, ""}; });

  router.add_error_handler(Request::Failure::UNAUTHORIZEDMETHOD,
                           [](Request _req) -> Response { return {{"", 5}, {}, ""}; });

  router.add_error_handler(Request::Failure::MALFORMED,
                           [](Request _req) -> Response { return {{"", 6}, {}, ""}; });

  failed3 = router.respond({Request::Failure::METHOD}).cmd.status_code != 0;
  failed3 = router.respond({Request::Failure::TRAILING}).cmd.status_code != 1;
  failed3 = router.respond({Request::Failure::SIZE}).cmd.status_code != 2;
  failed3 = router.respond({Request::Failure::PATH}).cmd.status_code != 3;
  failed3 = router.respond({Request::Failure::LENGTH}).cmd.status_code != 4;
  failed3 = router.respond({Request::Failure::UNAUTHORIZEDMETHOD}).cmd.status_code != 5;
  failed3 = router.respond({}).cmd.status_code != 6;
  failed3 = router.respond({Request::Failure::WRONGPATH}).cmd.status_code != 500;

  if (failed1 || failed2 || failed3) {
    ERR("router test failed");
    return;
  }

  LOG("router test passed");
  return;
}
