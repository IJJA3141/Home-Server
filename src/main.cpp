#include <iostream>

#include "config/config.hpp"

int main(int _argc, char *_argv[])
{
  Parser parser(_argc, _argv);

  parser.parse({"--help", "-h"}, []() {
    std::cout << "help..." << std::endl;
    exit(1);
  });

  parser.parse<std::string>({"--config", "-c"}, [](auto path) { std::cout << path << std::endl; });
  parser.parse<int>({"--config", "-c"}, [](auto path) { std::cout << path << std::endl; });

  return 0;
}
