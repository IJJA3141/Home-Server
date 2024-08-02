#include "../option/option.hpp"
#include "../log.hpp"

namespace Test
{
bool options()
{
  int i = 0;
  char *argv[] = {"path", "-ot10", "--four", "--five=10",  "--six",
                  "10",   "-s",    "10",     "subcommand", "--three"};

  Parser parser(10, argv, {"subcommand"},
                {
                    {'o', "one", [&] { i++; }},
                    {'t', "two", [&](int _) { i += _; }},
                    {-1, "three", [&] { i--; }},
                    {'f', "four", [&] { i++; }},
                    {-1, "five", [&](int _) { i += _; }},
                    {-1, "six", [&](int _) { i += _; }},
                    {'s', "seven", [&](int _) { i += _; }},
                });

  if (i == 42) {
    LOG("option test passed")
    return true;
  } else {
    ERR("option test failed")
    ERR("i: " << i)
    return false;
  }
};

bool subcommand()
{
  int i = 0;
  char *argv[] = {"path", "-ot10", "--four", "--five=10", "subcommand", "--six", "10", "-s", "10"};

  Parser parser(9, argv, {"subcommand"},
                {
                    {'o', "one", [&] { i++; }},
                    {'t', "two", [&](int _) { i += _; }},
                    {-1, "three", [&] { i--; }},
                    {'f', "four", [&] { i++; }},
                    {-1, "five", [&](int _) { i += _; }},
                });

  parser.parse({
      {-1, "six", [&](int _) { i += _; }},
      {'s', "seven", [&](int _) { i += _; }},
  });

  if (i == 42) {
    LOG("subcommand test passed")
    return true;
  } else {
    ERR("subcommand test failed")
    return false;
  }
};

} // namespace Test
