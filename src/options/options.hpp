#pragma once

#include <functional>
#include <string>

struct Option {
  enum Type { ARG, NOARG };
  Type type;
  std::function<void(std::string)> func;
  std::string longName;
  int shortName;
};

class Parser
{
private:
  std::vector<std::string> post_;
  std::vector<std::string> pre_;
  std::string scmd_;

public:
  Parser(std::vector<std::string> _scmds, std::vector<std::string> _argv);

  template <size_t S> void parse(Option _options[S]);
  template <size_t S> void commandParse(Option _options[S]);

  void gerErr();
private:

};
