#include "../src/network/http.hpp"
#include "test.hpp"
#include <cassert>
#include <string>
#include <vector>

bool split(const std::string _separator, const std::string_view _str,
           std::string &_left, std::string &_right) {
  size_t mid = _str.find(_separator);

  if (mid == _str.npos)
    return false;

  _left = _str.substr(0, mid++);
  _right = _str.substr(mid);
  return true;
};

void parse_cookies(http::Request::Failure &_failure,
                   std::vector<std::string> &_out, std::string _cookies) {
  size_t pos = 0, npos;
  std::string name;
  std::string value;

  while ((npos = _cookies.find(";", pos)) != _cookies.npos) {
    if (!split("=", _cookies.substr(pos, npos - pos), name, value)) {
      _failure = http::Request::Failure::HEADER;
      return;
    };

    pos = npos + 2;
    _out.push_back(name);
    _out.push_back(value);
  }

  if (!split("=", _cookies.substr(pos), name, value)) {
    _failure = http::Request::Failure::HEADER;
    return;
  };

  _out.push_back(name);
  _out.push_back(value);
  return;
}

int __test_split(int _argc, char *_argv[]) {
  std::string left;
  std::string right;

  std::string v0 = "PHPSESSID=298zf09hf012fh2";
  assert(split("=", v0, left, right));
  assert(left == "PHPSESSID");
  assert(right == "298zf09hf012fh2");

  v0 = "PHPSESSID=298zf09hf012fh2";
  assert(!split("&", v0, left, right));

  return 0;
}

int __test_cookies(int _argc, char *_argv[]) {
  http::Request::Failure v0 = http::Request::Failure::NONE;
  std::vector<std::string> v1;
  std::string v2 = "PHPSESSID=298zf09hf012fh2; csrftoken=u32t4o3tb3gg43; _gat=1";

  parse_cookies(v0, v1, v2);
  assert(v1[0] == "PHPSESSID");
  assert(v1[1] == "298zf09hf012fh2");
  assert(v1[2] == "csrftoken");
  assert(v1[3] == "u32t4o3tb3gg43");
  assert(v1[4] == "_gat");
  assert(v1[5] == "1");
  assert(v0 == http::Request::Failure::NONE);
  v1.clear();

  v2 = "PHPSESSID=298zf09hf012fh2";
  parse_cookies(v0, v1, v2);
  assert(v1[0] == "PHPSESSID");
  assert(v1[1] == "298zf09hf012fh2");
  assert(v0 == http::Request::Failure::NONE);

  v2 = "PHPSESSIDs298zf09hf012fh2";
  parse_cookies(v0, v1, v2);
  assert(v0 == http::Request::Failure::HEADER);

  return 0;
}

int __test_request(int _argc, char *_argv[]){
  bool failed = false;

  http::Request req0(
      "POST /over/there?user=IJJA&quoi=coupb&2+2=5 HTTP/1.1\r\nHost: google.com\r\nUser-Agent: "
      "curl/8.9.1\r\nAccept: "
      "*/*\r\nContent-Length: 4\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\ntest",
      true);

  failed = failed || req0.cmd.method != http::Method::POST;
  failed = failed || req0.cmd.path != std::vector<std::string>({"/over", "/there"});
  failed = failed || req0.cmd.protocol != "HTTP/1.1";

  failed = failed || req0.body != "test";

  failed = failed || req0.headers["Content-Length"] != "4";
  failed = failed || req0.headers["Accept"] != "*/*";
  failed = failed || req0.headers["Content-Type"] != "application/x-www-form-urlencoded";
  failed = failed || req0.headers["Host"] != "google.com";

  failed = failed || req0.url_params["user"] != "IJJA";
  failed = failed || req0.url_params["quoi"] != "coupb";

  http::Request req1(
      "POST /over/there HTTP/1.1\r\nHost: google.com\r\nUser-Agent: "
      "curl/8.9.1\r\nAccept: "
      "*/*\r\nContent-Length: 4\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\ntest",
      true);

  failed = failed || req1.url_params.size() > 0;

  http::Request req2(
      "POST /over/there?user=IJJA HTTP/1.1\r\nHost: google.com\r\nUser-Agent: "
      "curl/8.9.1\r\nAccept: "
      "*/*\r\nContent-Length: 4\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\ntest",
      true);

  failed = failed || req2.url_params["user"] != "IJJA";

  http::Request req3(
      "GET /login HTTP/1.1\r\nHost: localhost\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; "
      "x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36 "
      "GLS/100.10.9939.100\r\nAccept: */*\r\nAccept-Language: en-US,en;q=0.5\r\nAccept-Encoding: "
      "gzip, deflate, br, zstd\r\nReferer: https://localhost/login\r\nAuthorization: Basic "
      "Og==\r\nDNT: 1\r\nSec-GPC: 1\r\nConnection: keep-alive\r\nSec-Fetch-Dest: "
      "empty\r\nSec-Fetch-Mode: cors\r\nSec-Fetch-Site: same-origin\r\nsec-ch-ua-platform: "
      "\"Windows\"\r\nsec-ch-ua: \"Google Chrome\";v=\"125\", \"Chromium\";v=\"125\", "
      "\"Not=A?Brand\";v=\"24\"\r\nsec-ch-ua-mobile: ?0\r\nPriority: u=0\"",
      false);

  failed = failed || req3.cmd.method != http::Method::GET;
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
    return 1;
  }

  return 0;
}
