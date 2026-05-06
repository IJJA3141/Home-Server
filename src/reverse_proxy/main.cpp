#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../protocol/http/http.hpp"
#include "../protocol/protocol.hpp"
#include "reverse_proxy.hpp"

static_assert(protocol::Protocol<protocol::HTTP>, "FAILED?");

int main(void)
{
  ipc::TransportClient<protocol::HTTP> mihon_sync(LOCAL_HOST, MIHON_SYNC_PORT);
  mihon_sync.connect();

  TlsServer tls("", 443, CERT_PATH, KEY_PATH, [&](protocol::HTTP::Request _request) -> protocol::HTTP::Response {
    return mihon_sync.transmit(_request);
    return protocol::HTTP::standard_response(404);
  });

  tls.listen();

  return 0;
}
