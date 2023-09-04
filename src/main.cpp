#include "./tcp-server/tcp-server.hpp"
#include "./tls-server/tls-server.hpp"
#include "./params/params.hpp"

#include <thread>

int main(int _argc, char *_argv[]) {
  Params params = Params(_argc, _argv);

  http::TcpServer httpServer = http::TcpServer();
  http::TlsServer httpsServer =
      http::TlsServer(params.cert.c_str(), params.key.c_str());

  httpServer.bind((char *)params.http.c_str());
  httpsServer.bind((char *)params.https.c_str());

  std::thread http(&http::TcpServer::listen, httpServer);
  std::thread https(&http::TcpServer::listen, httpsServer);

  http.join();
  https.join();

  return 0;
}
