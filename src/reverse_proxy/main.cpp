#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../protocol/http/http.hpp"
#include "../protocol/protocol.hpp"
#include "reverse_proxy.hpp"
#include <print>

static_assert(protocol::Protocol<protocol::HTTP>, "FAILED?");

int main(void)
{
  ipc::TransportClient<protocol::HTTP> mihon_sync(LOCAL_HOST, MIHON_SYNC_PORT);
  mihon_sync.connect();

  TcpServer tcp(REVERSE_PROXY_PORT, 80, [&](protocol::HTTP::Request _request) -> protocol::HTTP::Response {
    // if (_request.headers["host"] == "mihon") return mihon_sync.transmit(_request);
    // return {protocol::HTTP::Version::HTTP_11, 200, {{"Content-length", "1"}, {"content-type", "text/html"}}, "?"};
    return mihon_sync.transmit(_request);
  });

  tcp.listen();

  // TlsServer tls(REVERSE_PROXY_PORT, 443, CERT_PATH, KEY_PATH,
  //               [&](protocol::HTTP::Request _request) -> protocol::HTTP::Response {
  //                 if (_request.headers["host"] == "mihon") return mihon_sync.transmit(_request);
  //                 return protocol::HTTP::standard_response(404);
  //               });
  //
  // tls.listen();

  return 0;
}
