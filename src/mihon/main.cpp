#include "../config.hpp"
#include "../ipc/transport.hpp"
#include <print>

using Response = protocol::HTTP::Response;
using Request = protocol::HTTP::Request;

int main(void)
{
  ipc::TransportServer<protocol::HTTP> server(MIHON_SYNC_PORT, [](Request _request) {
    std::println("{}", (std::string)_request);

    Response res;
    res.version = protocol::HTTP::HTTP_11;
    res.status = 200;
    res.headers = {};
    res.body = "pong";

    std::println("み -> {}", std::string(res));

    return res;
  });
  server.listen();

  return 0;
}
