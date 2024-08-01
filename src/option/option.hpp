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
  std::vector<std::string> pre_;
  std::vector<std::string> post_;

public:
  std::string scmd;

public:
  Parser(int _argc, char *_argv[], std::vector<std::string> _scmds, std::vector<Option> _opts);

  void parse(std::vector<Option> _opts);
};
