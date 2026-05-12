#include "../../src/routing/routing.hpp"
#include "../test.hpp"
#include <exception>

using Method = protocol::HTTP::Method;
using Middleware = protocol::Middleware<protocol::HTTP>;
using Handler = protocol::Handler<protocol::HTTP>;
using Request = protocol::HTTP::Request;
using Response = protocol::HTTP::Response;

int protocol_router(int argc, char* argv[])
{

  int _flag = 0, _i = 0, _j = 0;
  bool _exception_caught = false;

  SECTION("Router assertions")
  {
    REQUIRE_THROW("", std::exception&, {
      assert_path("");
    })
  }

  SECTION("Router basic routing")
  {
    Router r;
    r.add(Method::GET, "", [](Request&) -> Response { return {}; });
  }
}
