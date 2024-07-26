#include <iostream>

#include "config/config.hpp"

void help()
{
  std::cout << "help" << std::endl;
  exit(1);
};

int main(int _argc, char *_argv[])
{
  std::vector<Arg> args;
  bool wasKey = false;
  Config config;

  // start at 1 to skip path
  for (size_t i = 1; i < _argc; i++) {
    if (_argv[i][0] != '-' && wasKey) {
      args[args.size() - 1].arg = new std::string(_argv[i]);
    } else {
      args.push_back({_argv[i], nullptr});
      wasKey = true;
    }
  }

  parse(args, {
                  {"-h", help},
                  {"--help", help},
                  {"-v", [&] { config.verbose = true; }},
                  {"--verbose", [&] { config.verbose = true; }},
                  {"-c", [&](std::string _) { config.load(_); }},
                  {"--config", [&](std::string _) { config.load(_); }},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
                  {"", [] {}},
              });

  return 0;
}
