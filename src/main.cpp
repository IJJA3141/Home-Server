#include "./tcp-server/tcp-server.hpp"
#include "./tls-server/tls-server.hpp"

#include <thread>

char *pathCert = "./ssl/cert.pem";
char *pathKey = "./ssl/key.pem";
char *httpPort = "80";
char *httpsPort = "443";

void parse(int _argc, char *_argv[]) {
  if (_argc == 2)
    std::cout << "Available parameters:\n\
                  --path-cert: set a path to the certificate\n\
                  --path-key: set a path to the certificate key\n\
                  --port-http: set a port for the http server\n\
                  --port-https: set a port for the https server"
              << std::endl;
  else {
    for (int i = 1; i < _argc - 1; i += 2) {
      if (strcmp("--path-cert", _argv[i]) == 0) {
        pathCert = _argv[i + 1];
        std::cout << "Path to certificate has been changed to: " << pathCert
                  << std::endl;
      } else if (strcmp("--path-key", _argv[i]) == 0) {
        pathKey = _argv[i + 1];
        std::cout << "Path to key has been changed to: " << pathKey
                  << std::endl;
      } else if (strcmp("--port-http", _argv[i]) == 0) {
        httpPort = _argv[i + 1];
        std::cout << "Port of http server has been changed to: " << httpPort
                  << std::endl;
      } else if (strcmp("--port-https", _argv[i]) == 0) {
        httpsPort = _argv[i + 1];
        std::cout << "Port of https server has been changed to: " << httpsPort
                  << std::endl;
      } else {
        std::cout << "Wrong parameters have been ignored.\nFor more "
                     "informations, type --help."
                  << std::endl;
      }
    }
  }
};

int main(int _argc, char *_argv[]) {
  parse(_argc, _argv);

  http::TcpServer httpServer = http::TcpServer();
  http::TlsServer httpsServer = http::TlsServer(pathCert, pathKey);

  httpServer.bind(httpPort);
  httpsServer.bind(httpsPort);

  std::thread http(&http::TcpServer::listen, httpServer);
  std::thread https(&http::TcpServer::listen, httpsServer);

  http.join();
  https.join();

  return 0;
}
