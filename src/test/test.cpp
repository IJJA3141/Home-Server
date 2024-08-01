#include "../option/option.hpp"

bool testOptions()
{
  int i = 0;
  char *argv[] = {"path", "-ot10", "--four", "--five=10",  "six",
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

  return (parser.scmd == "subcommand" && i == 42);
};
