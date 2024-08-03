#pragma once

#include <functional>
#include <string>
#include <vector>

struct Option {
  std::function<void(std::string)> func;
  std::string longName;
  int shortName;
  bool hasArg;

  Option(int _shortName, std::string _longName, std::function<void(std::string)> _func);
  Option(int _shortName, std::string _longName, std::function<void(void)> _func);
  Option(int _shortName, std::string _longName, std::function<void(int)> _func);
};

class Parser
{
private:
  std::vector<std::string> argv_;
  std::vector<std::string> subArgv_;
  std::vector<Option> optv_;
  std::string arg_;

public:
  std::string scmd;

public:
  Parser(int _argc, char *_argv[], std::vector<std::string> _scmdv);

  void add(std::vector<Option> _optv);
  void parse();
  void subParse();

private:
  void parse(std::vector<std::string> &_argv);

  void shortArg();
  void longArg(std::vector<std::string>& _argv);
};
