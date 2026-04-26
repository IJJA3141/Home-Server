#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../networking/http.hpp"
#include "../networking/server.hpp"
#include "../protocol/http/http.hpp"
#include "../protocol/protocol.hpp"
#include <print>

static_assert(protocol::Protocol<protocol::HTTP>, "FAILED?");

int main(void)
{
  ipc::TransportClient<protocol::HTTP> mihon_sync(LOCAL_HOST, MIHON_SYNC_PORT);
  mihon_sync.connect();

  Tls tls(CERT_PATH, KEY_PATH, 443, [&](http::Request _request) -> http::Response {
    _request.headers["user_id"] = "what a nice uuid";

    auto s = std::string(_request);
    std::println("TSL <- {}", s);
    return {}; // bug ??
  });

  tls.listen();

  return 0;
}
