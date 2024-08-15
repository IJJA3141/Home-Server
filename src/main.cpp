#include "./parsing/parsing.hpp"

int main(int _argc, char *_argv[])
{
  Config config;

  Parser parser(_argc, _argv, {});

  parser.add({{'2', "", [] {}}});
  parser.parse();

  return 0;
}
