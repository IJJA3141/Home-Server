#include "../log.hpp"
#include "../parsing/parsing.hpp"

namespace Test
{
bool options()
{
  int i = 0;
  char *argv[] = {"path", "-ot10", "--four", "--five=10",  "--six",
                  "10",   "-s",    "10",     "subcommand", "--three"};

  Parser parser(10, argv, {"subcommand"});

  parser.add({
      {'o', "one", [&] { i++; }},
      {'t', "two", [&](int _) { i += _; }},
      {-1, "three", [&] { i--; }},
      {'f', "four", [&] { i++; }},
      {-1, "five", [&](int _) { i += _; }},
      {-1, "six", [&](int _) { i += _; }},
      {'s', "seven", [&](int _) { i += _; }},
  });

  parser.parse();

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
  bool brok = false;
  char *argv[] = {"path", "-ot10", "--four", "--five=10", "subcommand", "--six", "10", "-s", "10"};

  Parser parser(9, argv, {"subcommand"});
  if (parser.scmd != "subcommand") brok = true;
  Parser parser2(9, argv, {"sommand"});
  if (parser2.scmd != "") brok = true;

  if (!brok) {
    LOG("subcommand test passed")
    return true;
  } else {
    ERR("subcommand test failed")
    ERR("parser 1 subcommand: " << parser.scmd);
    ERR("parser 2 subcommand: " << parser2.scmd);
    return false;
  }
}

bool suboptions()
{
  int i = 0;
  char *argv[] = {"path", "-ot10", "--four", "--five=10", "subcommand", "--six", "10", "-s", "10"};

  Parser parser(9, argv, {"subcommand"});

  parser.add({
      {'o', "one", [&] { i++; }},
      {'t', "two", [&](int _) { i += _; }},
      {-1, "three", [&] { i--; }},
      {'f', "four", [&] { i++; }},
      {-1, "five", [&](int _) { i += _; }},
  });

  parser.parse();

  parser.add({
      {-1, "six", [&](int _) { i += _; }},
      {'s', "seven", [&](int _) { i += _; }},
  });

  parser.subParse();

  if (i == 42) {
    LOG("suboption test passed")
    return true;
  } else {
    ERR("suboption test failed")
    return false;
  }
};

} // namespace Test
