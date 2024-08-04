#include "test/test.hpp"

int main(int _argc, char *_argv[])
{
  Test::options();
  Test::subcommand();
  Test::suboptions();

  return 0;
}
