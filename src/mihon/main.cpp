#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../routing/routing.hpp"
#include <print>
#include <string>

using Method = protocol::HTTP::Method;
using Response = protocol::HTTP::Response;
using Request = protocol::HTTP::Request;
using Handler = protocol::Handler<protocol::HTTP>;

int main(void)
{
  Router r;
  r.add(Method::GET, "/", [](Request&) -> Response { return {}; });
  r.add(Method::GET, "/wild/*", [](Request&) -> Response { return {}; });
  r.add(Method::GET, "/param/[user]/consumer/", [](Request&) -> Response { return {}; });
  r.add(Method::GET, "/superpath", [](Request&) -> Response { return {}; });
  r.add(Method::GET, "/superpath/a", [](Request&) -> Response { return {}; });
  r.add(Method::DELETE, "/superpath/long/path/with/a/lot/of/sub/path", [](Request&) -> Response { return {}; });
  r.add(Method::GET, "/superpath/b", [](Request&) -> Response { return {}; });
  r.add(
      Method::PUT, "/", [](Request&, Handler) -> Response { return {}; }, [](Request&) -> Response { return {}; });

  std::print("{}", std::string(r));

  // ipc::TransportServer<protocol::HTTP> server(
  //     LOCAL_HOST, MIHON_SYNC_PORT,
  //     [](Request _request) {
  //       std::println("{}", (std::string)_request);
  //
  //       Response res;
  //       res.version = protocol::HTTP::Version::HTTP_11;
  //       res.status = 200;
  //       res.headers = {};
  //       res.body = "pong";
  //
  //       return res;
  //     },
  //     "");
  // server.listen();

  return 0;
}
