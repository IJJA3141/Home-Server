#include "log.hpp"
#include "server/parser.hpp"
#include "server/router.hpp"
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

  if (res1.headers["Host:"] != "google.com") {
    ERR("wrong header value for host: " << res1.headers["Host:"]);
    failed = true;
  }
  res1.headers.erase("Host:");

  if (res1.headers["User-Agent:"] != "curl/8.9.1") {
    ERR("wrong header value for user-agent: " << res1.headers["User-Agent:"]);
    failed = true;
  }
  res1.headers.erase("User-Agent:");

  if (res1.headers["Accept:"] != "*/*") {
    ERR("wrong header value for accept: " << res1.headers["Accept:"]);
    failed = true;
  }
  res1.headers.erase("Accept:");

  if (res1.headers.size() > 0) {
    for (const auto &key_val : res1.headers)
      ERR(key_val.first << " | " << key_val.second);

    failed = true;
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

  if (res2.headers["Host:"] != "www.google.com") {
    ERR("wrong header value for host: " << res2.headers["Host:"]);
    failed = true;
  }
  res2.headers.erase("Host:");

  if (res2.headers["User-Agent:"] != "curl/8.9.1") {
    ERR("wrong header value for user-agent: " << res2.headers["User-Agent:"]);
    failed = true;
  }
  res2.headers.erase("User-Agent:");

  if (res2.headers["Accept:"] != "*/*") {
    ERR("wrong header value for accept: " << res2.headers["Accept:"]);
    failed = true;
  }
  res2.headers.erase("Accept:");

  if (res2.headers["Content-Length:"] != "19") {
    ERR("wrong header value for content-length: |" << res2.headers["Content-Length:"] << "|");
    failed = true;
  }
  res2.headers.erase("Content-Length:");

  if (res2.headers["Content-Type:"] != "application/x-www-form-urlencoded") {
    ERR("wrong header value for content-type: |" << res2.headers["Content-Type:"] << "|");
    failed = true;
  }
  res2.headers.erase("Content-Type:");

  if (res2.headers.size() > 0) {
    for (const auto &key_val : res2.headers)
      ERR(key_val.first << " | " << key_val.second << " should not be in the headers.");

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
    if (str == "\r") {
      PRINT("|r|");
    } else if (str == "\n") {
      PRINT("|n|");
    } else if (str == "\r\n") {
      PRINT("|rn|");
    } else {
      PRINT("|" << str << "|");
    }
  }

  return;
}

void path()
{
  std::string str1 = "/over/there/[user]/end/[param1]/[param2]/path";
  std::string str2 = "/[user]/end/[param1]/[param2]";
  std::string str3 = "/over/end/param";
  Path path1 = Path(str1);
  Path path2 = Path(str2);
  Path path3 = Path(str3);

  PRINT(str1);
  PRINT(path1.match);
  for (const auto &pair : path1.skip)
    PRINT(pair.first << " | " << pair.second);

  PRINT(str2);
  PRINT(path2.match);
  for (const auto &pair : path2.skip)
    PRINT(pair.first << " | " << pair.second);

  PRINT(str3);
  PRINT(path3.match);
  for (const auto &pair : path3.skip)
    PRINT(pair.first << " | " << pair.second);

  return;
}

}; // namespace test
