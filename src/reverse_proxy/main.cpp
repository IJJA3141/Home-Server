#include "../config.hpp"
#include "../ipc/transport.hpp"
#include "../networking/http.hpp"
#include "../networking/server.hpp"
#include "../protocol/http.hpp"
#include "../protocol/protocol.hpp"
#include <print>

#define LOCAL_HOST "127.0.0.0"

static_assert(Protocol<protocol::HTTP>, "FAILED?");

int main(void)
{
  ipc::TransportClient<protocol::HTTP> mihon_sync(MIHON_SYNC_PORT, LOCAL_HOST);
  mihon_sync.connect();

  Tls tls(CERT_PATH, KEY_PATH, 443, [&](http::Request _request) -> http::Response {
    _request.headers["user_id"] = "what a nice uuid";

    auto s = std::string(_request);
    std::println("TSL <- {}", s);

    protocol::HTTP::Request r(s);
    std::println("{} <- TLS", std::string(r));

    auto res = (std::string)mihon_sync.transmit(r);
    std::println(" --- {}", res);

    return http::parse_response(res); // bug ??
  });

  tls.listen();

  return 0;
}
