#include "option/option.hpp"
#include "test/test.hpp"

int main(int _argc, char *_argv[])
{
  if (!Test::options() || !Test::subcommand()) return 1;

  return 0;
}
