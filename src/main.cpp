#include "./parser/parser.hpp"
#include "./tcp-server/tcp-server.hpp"
#include "./tls-server/tls-server.hpp"

#include <cstring>
#include <string>
#include <thread>

<<<<<<< HEAD
char *pathCert = (char *)"./ssl/cert.pem";
char *pathKey = (char *)"./ssl/key.pem";
char *httpPort = (char *)"80";
char *httpsPort = (char *)"443";
=======
char *pathCert = "/etc/letsencrypt/live/ijja.dev/cert.pem";
char *pathKey = "./etc/letsencrypt/live/ijja.dev/privkey.pem";
char *httpPort = "80";
char *httpsPort = "443";
>>>>>>> 2da9a24446a5b0fd3263415bfc9966b7863ca15f

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
  /*
  parse(_argc, _argv);

  http::TcpServer httpServer = http::TcpServer();
  http::TlsServer httpsServer = http::TlsServer(pathCert, pathKey);

  httpServer.bind(httpPort);
  httpsServer.bind(httpsPort);

  std::thread http(&http::TcpServer::listen, httpServer);
  std::thread https(&http::TcpServer::listen, httpsServer);

  http.join();
  https.join();
  */

  char *m1 = (char *)"\
  GET / HTTP/1.1\n\
  Host: 0.0.0.0:37121\n\
  User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:102.0) Gecko/20100101 Firefox/102.0\n\
  Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\n\
  Accept-Language: en-US,en;q=0.5\n\
  Accept-Encoding: gzip, deflate\n\
  Connection: keep-alive\n\
  Upgrade-Insecure-Requests: 1";

  char *m2 = (char *)"\
  POST / HTTP/1.1\n\
  Host: 192.168.1.123:35271\n\
  User-Agent: curl/8.0.1\n\
  Accept: * /*\n\
  Content-Type: application/json\n\
  Content-Length: 15\n\
  \n\
  {'fatya':'333'}";

  char *m3 = (char *)"\
  GET /test/id=4324hkj24h32hkldshjklfdsalhl&q=jfds+jkds HTTP/1.1\n\
  Host: 192.168.1.123:53509\n\
  User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/117.0\n\
  Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\n\
  Accept-Language: en-US,en;q=0.5\n\
  Accept-Encoding: gzip, deflate\n\
  Connection: keep-alive\n\
  Upgrade-Insecure-Requests: 1";

  char *m4 = (char *)"\
  GET / HTTP/1.1\n\
  Host: 192.168.1.123:39843\n\
  User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/117.0\n\
  Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8\n\
  Accept-Language: en-US,en;q=0.5\n\
  Accept-Encoding: gzip, deflate, br\n\
  Connection: keep-alive\n\
  Upgrade-Insecure-Requests: 1\n\
  Sec-Fetch-Dest: document\n\
  Sec-Fetch-Mode: navigate\n\
  Sec-Fetch-Site: none\n\
  Sec-Fetch-User: ?1";

  http::Request req1 = http::Request(m1);
  // http::Request req2 = http::Request(m2);
  // http::Request req3 = http::Request(m3);
  // http::Request req4 = http::Request(m4);

  std::cout << "\n\n" << req1.body << "\n"
            << req1.path << "\n"
            << req1.upgrade << "\n"
            << req1.contentSize << "\n"
            << req1.body << "\n\n"
            << std::endl;
  // std::cout << req2.body << "\n" << req2.path << "\n" << req2.upgrade <<
  // "\n"<< req2.contentSize << "\n"<< req2.body << "\n\n"<< std::endl;
  // std::cout << req3.body << "\n"           << req3.path << "\n"            <<
  // req3.upgrade << "\n"            << req3.contentSize << "\n"         <<
  // req3.body << "\n\n"            << std::endl; std::cout << req4.body << "\n"
  // << req4.path << "\n"            << req4.upgrade << "\n"        <<
  // req4.contentSize << "\n"    << req4.body << "\n\n"           << std::endl;

  return 0;
}
