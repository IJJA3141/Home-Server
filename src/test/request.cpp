#include "../server/reqres.hpp"
#include "test.hpp"

void test::request()
{
  bool failed = false;

  Request req0(
      "POST /over/there?user=IJJA&quoi=coupb&2+2=5 HTTP/1.1\r\nHost: google.com\r\nUser-Agent: "
      "curl/8.9.1\r\nAccept: "
      "*/*\r\nContent-Length: 4\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\ntest",
      Client::Type::SSL);

  failed = failed || req0.cmd.method != Method::POST;
  failed = failed || req0.cmd.path != std::vector<std::string>({"/over", "/there"});
  failed = failed || req0.cmd.protocol != "HTTP/1.1";

  failed = failed || req0.body != "test";

  failed = failed || req0.headers["Content-Length"] != "4";
  failed = failed || req0.headers["Accept"] != "*/*";
  failed = failed || req0.headers["Content-Type"] != "application/x-www-form-urlencoded";
  failed = failed || req0.headers["Host"] != "google.com";

  failed = failed || req0.url_params["user"] != "IJJA";
  failed = failed || req0.url_params["quoi"] != "coupb";

  Request req1(
      "POST /over/there HTTP/1.1\r\nHost: google.com\r\nUser-Agent: "
      "curl/8.9.1\r\nAccept: "
      "*/*\r\nContent-Length: 4\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\ntest",
      Client::Type::SSL);

  failed = failed || req1.url_params.size() > 0;

  Request req2(
      "POST /over/there?user=IJJA HTTP/1.1\r\nHost: google.com\r\nUser-Agent: "
      "curl/8.9.1\r\nAccept: "
      "*/*\r\nContent-Length: 4\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\ntest",
      Client::Type::SSL);

  failed = failed || req2.url_params["user"] != "IJJA";

  Request req3(
      "GET /login HTTP/1.1\r\nHost: localhost\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; "
      "x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36 "
      "GLS/100.10.9939.100\r\nAccept: */*\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: "
      "gzip, deflate, br, zstd\r\nReferer: https://localhost/login\r\nAuthorization: Basic "
      "Og==\r\nDNT: 1\r\nSec-GPC: 1\r\nConnection: keep-alive\r\nSec-Fetch-Dest: "
      "empty\r\nSec-Fetch-Mode: cors\r\nSec-Fetch-Site: same-origin\r\nsec-ch-ua-platform: "
      "\"Windows\"\r\nsec-ch-ua: \"Google Chrome\";v=\"125\", \"Chromium\";v=\"125\", "
      "\"Not=A?Brand\";v=\"24\"\r\nsec-ch-ua-mobile: ?0\r\nPriority: u=0\"",
      Client::Type::STANDARD);

  failed = failed || req3.cmd.method != Method::GET;
  failed = failed || req3.cmd.path != std::vector<std::string>({"/login"});
  failed = failed || req3.cmd.protocol != "HTTP/1.1";

  failed =
      failed || req3.headers["sec-ch-ua"] !=
                    "\"Google Chrome\";v=\"125\", \"Chromium\";v=\"125\", \"Not=A?Brand\";v=\"24\"";
  failed = failed || req3.headers["Referer"] != "https://localhost/login";
  failed = failed || req3.headers["Authorization"] != "Basic Og==";
  failed = failed || req3.headers["Accept-Language"] != "en-US,en;q=0.5";

  if (failed) {
    ERR("request test failed");
    return;
  }

  LOG("request test passed");
  return;
}
