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

  if (failed) {
    ERR("request test failed");
    return;
  }

  LOG("request test passed");
  return;
}
