#include "./params/params.hpp"
#include "./parser/parser.hpp"
#include "./tcp-server/tcp-server.hpp"
#include "./tls-server/tls-server.hpp"

#include <iostream>

int main(int _argc, char *_argv[]) {
  for (int i = 0; i < _argc; i++)
    std::cout << _argv[i] << std::endl;

  Params params = Params(_argc, _argv);

  http::TcpServer httpServer = http::TcpServer("Http server");
  http::TlsServer httpsServer =
      http::TlsServer(params.cert.c_str(), params.key.c_str(), "Https server");

  httpServer.bind(params.http.c_str());
  httpsServer.bind(params.https.c_str());

  httpsServer.add(http::Method::GET, "root", [](void *_pVoid) { return; });

  std::thread http(&http::TcpServer::listen, &httpServer);
  std::thread https(&http::TlsServer::listen, &httpsServer);

  return 0;
}
