#include "../config.hpp"
#include "../ipc/transport/transport.hpp"
#include "../logger/logger.hpp"
#include "../protocol/http/http.hpp"
#include "reverse_proxy.hpp"

// static_assert(protocol::Protocol<protocol::HTTP>, "FAILED?");

int main(void)
{
  ipc::TransportClient<protocol::HTTP> mihon_sync(MIHON_IP, MIHON_SYNC_PORT);
  mihon_sync.connect();

  TcpServer tcp(REVERSE_PROXY_PORT, 80, [&](protocol::HTTP::Request _request) -> protocol::HTTP::Response {
    // if (_request.headers["host"] == "mihon") return mihon_sync.transmit(_request);
    // return {protocol::HTTP::Version::HTTP_11, 200, {{"Content-length", "1"}, {"content-type", "text/html"}},
    // "?"};
    
    auto res = mihon_sync.transmit(_request);
    Logger::debug("sending\n{}", std::string(res));


    return res;
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
