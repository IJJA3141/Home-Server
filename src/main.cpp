#include "./parser/parser.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

int main(int _argc, char *_argv[]) {
  char *a = (char *)""
                    "get /tap/tap/index.html HTTP/1.1\n"
                    "Host: 0.0.0.0:37121\n"
                    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:102.0) "
                    "Gecko/20100101 Firefox/102.0\n"
                    "Accept: "
                    "text/html,application/xhtml+xml,application/"
                    "xml;q=0.9,image/avif,image/webp,*/*;q=0.8\n"
                    "Accept-Language: en-US,en;q=0.5\n"
                    "Accept-Encoding: gzip, deflate\n"
                    "Connection: keep-alive\n"
                    "Upgrade-Insecure-Requests: 1\n"
                    "";

  char *aa = (char *)""
                     "PST / HTTP/1.1\n"
                     "Host: 192.168.1.123:35271\n"
                     "User-Agent: curl/8.0.1\n"
                     "Accept: */*\n"
                     "Content-Type: application/json\n"
                     "Content-Length: 15\n"
                     "\n"
                     "{'fatya' : '333'}\n"
                     "";

  char *aaa =
      (char
           *)""
             "GET /test/?id=4324hkj24h32hkldshjklfdsalhl&q=jfds+jkds HTTP/1.1\n"
             "Host: 192.168.1.123:53509\n"
             "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) "
             "Gecko/20100101 Firefox/117.0\n"
             "Accept: "
             "text/html,application/xhtml+xml,application/xml;q=0.9,image/"
             "avif,image/"
             "webp,*/*;q=0.8\n"
             "Accept-Language: en-US,en;q=0.5\n"
             "Accept-Encoding: gzip, deflate\n"
             "Connection: keep-alive\n"
             "Upgrade-Insecure-Requests: 1\n"
             "";

  std::vector<char *> v = {a, aa, aaa};
  std::vector<http::Request> vv = {};

  for (char *_ : v)
    vv.push_back(http::parse(std::string(_)));

  for (http::Request _ : vv)
    std::cout << "Method: " << _.method << "\nUri: " << _.uri
              << "\nFile: " << _.file << "\nArg: " << _.arg
              << "\nVersion: " << _.version
              << "\n\nHeader______\nHost: " << _.header.host
              << "\nagent: " << _.header.agent
              << "\naccept: " << _.header.accept << "\nlang: " << _.header.lang
              << "\nencoding: " << _.header.encoding
              << "\nconnection: " << _.header.connection
              << "\nupgrade: " << _.header.upgrade
              << "\ntype: " << _.header.type << "\nlength: " << _.header.length
              << "\n\n\n"
              << std::endl;

  return 0;
}

/*
#include "./params/params.hpp"
#include "./tcp-server/tcp-server.hpp"
#include "./tls-server/tls-server.hpp"

#include <thread>

int main(int _argc, char *_argv[]) {
  Params params = Params(_argc, _argv);

  http::TcpServer httpServer = http::TcpServer("Http server");
  http::TlsServer httpsServer =
      http::TlsServer(params.cert.c_str(), params.key.c_str(), "Https server");

  httpServer.bind(params.http.c_str());
  httpsServer.bind(params.https.c_str());

  std::thread http(&http::TcpServer::listen, &httpServer);
  std::thread https(&http::TlsServer::listen, &httpsServer);

  return 0;
}
*/
