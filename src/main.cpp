#include "./params/params.hpp"

#include <functional>
#include <iostream>
#include <map>

int main(int _argc, char *_argv[]) {
  Parser parser = Parser();
  bool save = true;
  char *httpPort = (char *)"";
  char *httpsPort = (char *)"";
  char *certPath = (char *)"";
  char *keyPath = (char *)"";

  const char *msg =
      "Help page:\n"
      "-h --help                          shows commands.\n"
      "--no-save                          disable parameters saving.\n"
      "--cert-path <path to certificate>  set new path of certificate.\n"
      "--key-path <path to key>           set new path of certificate "
      "key.\n"
      "--http-port <port>                 http server will listen on new "
      "port.\n"
      "--https-port <port>                https server will listen on new "
      "port.\n";

  parser.add("-h", [&](char *_char) { std::cout << msg << std::endl; });
  parser.add("--help", [&](char *_char) { std::cout << msg << std::endl; });
  parser.add("--no-save", [&](char *_char) { save = false; });
  parser.add("--cert-path", [&](char *_char) { certPath = _char; });
  parser.add("--key-path", [&](char *_char) { keyPath = _char; });
  parser.add("--http-port", [&](char *_char) { httpPort = _char; });
  parser.add("--https-port", [&](char *_char) { httpsPort = _char; });

  std::cout << certPath << std::endl;
  std::cout << keyPath << std::endl;
  std::cout << httpPort << std::endl;
  std::cout << httpsPort << std::endl;

  std::function<void(char *_char)> b = [](char *_char) {
    std::cout << _char << std::endl;
    return;
  };

  std::map<const char, std::function<void(char *_char)>> a;
  a[*"test"] = b;
  a[*"test"]((char *)"test");

  return 0;
}
