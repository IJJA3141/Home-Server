#include "log.hpp"
#include "server/parser.hpp"
#include "server/server.hpp"

namespace test
{

void parse()
{
  std::string msg1 = "GET / HTTP/1.1\r\nHost: google.com\r\nUser-Agent: curl/8.9.1\r\nAccept: */*";

  std::string msg2 =
      "POST /home/test HTTP/1.1\r\nHost: www.google.com\r\nUser-Agent: curl/8.9.1\r\nAccept: "
      "*/*\r\nContent-Length: 19\r\nContent-Type: application/x-www-form-urlencoded";

  Message res1 = ::parse(msg1);
  Message res2 = ::parse(msg2);

  bool failed = false;

  PRINT("parse test 1:");
  if (res1.cmd.method != Method::GET) {
    ERR("failed to get http method.");
    failed = true;
  }

  if (res1.cmd.path != "/") {
    ERR("failed to get path");
    ERR("|" << res1.cmd.path << "|")
    failed = true;
  }

  if (res1.cmd.protocol != "HTTP/1.1") {
    ERR("failed to get protocol");
    failed = true;
  }

  for (const auto &pair : res1.headers) {
    PRINT(pair.first << " | " << pair.second << "\n");
  }

  if (!failed) {
    PRINT("test 1 passed");
  }

  failed = false;

  PRINT("parse test 2:");
  if (res2.cmd.method != Method::POST) {
    ERR("failed to get http method.");
    failed = true;
  }

  if (res2.cmd.path != "/home/test") {
    ERR("failed to get path");
    ERR("|" << res2.cmd.path << "|")
    failed = true;
  }

  if (res2.cmd.protocol != "HTTP/1.1") {
    ERR("failed to get protocol");
    failed = true;
  }

  if (!failed) {
    PRINT("test 2 passed");
  }

  return;
};

void stream()
{
  std::string str = "1234  567\r\n\r\n89a bc\ndef";

  Stream test(str);
  while (test >> str) {
    PRINT("|" << str << "|");
  }

  return;
}

}; // namespace test
