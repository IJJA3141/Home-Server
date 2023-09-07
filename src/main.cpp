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

  httpServer.Bind(params.http.c_str());
  httpsServer.Bind(params.https.c_str());

  httpsServer.Add(http::Method::GET, "root", [](void *_pVoid) { return; });

  std::thread http(&http::TcpServer::Listen, &httpServer);
  std::thread https(&http::TlsServer::Listen, &httpsServer);

  http.join();
  https.join();

  return 0;
}
