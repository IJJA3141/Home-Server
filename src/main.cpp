#include <iostream>
#include <string>

#include "config/config.hpp"

int main(int _argc, char *_argv[])
{
  Config config;
  // pas ouf
  Parser parser(_argc, _argv,
                {"--help", "-h", "--verbose", "-v", "--config", "-p", "--defautl-config", "-d",
                 "--http-port", "--https-port", "--key", "-k", "--cert", "-c", "--save", "-s",
                 "--reset-config", "-r"});

  parser.parse({"--help", "-h"}, []() {
    std::cout << "help" << std::endl;
    exit(1);
  });
  parser.parse({"--verbose", "-v"}, [&]() {
    std::cout << "verbose" << std::endl;
    config.verbose = true;
  });

  parser.parse({"--config", "-p"}, [&](std::string _configPath) {
    if (config.verbose) std::cout << "" << std::endl;
    config.configPath = _configPath;
  });

  config.load();

  parser.parse({"--defautl-config", "-d"}, [&]() {
    bool verbose = config.verbose;
    if (config.verbose) std::cout << "" << std::endl;
    config = Config();
    config.verbose = verbose;
  });

  parser.parse({"--http-port", ""}, [&](int _port) {
    if (config.verbose) std::cout << "" << std::endl;
    config.httpPort = _port;
  });
  parser.parse({"--https-port", ""}, [&](int _port) {
    if (config.verbose) std::cout << "" << std::endl;
    config.httpPort = _port;
  });
  parser.parse({"--key", "-k"}, [&](std::string _path) {
    if (config.verbose) std::cout << "" << std::endl;
    config.key = _path;
  });
  parser.parse({"--cert", "-c"}, [&](std::string _path) {
    if (config.verbose) std::cout << "" << std::endl;
    config.cert = _path;
  });
  parser.parse({"--save", "-s"}, [&]() { config.save(); });
  parser.parse({"--restore-config", "-r"}, []() { Config().save(); });

  return 0;
}
