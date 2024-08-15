#pragma once

#include <functional>
#include <string>
#include <vector>

struct Option {
  const std::function<void(std::string)> func;
  const std::string longName;
  const int shortName;
  const bool hasArg;

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

  void shortArg(std::vector<std::string> &_argv);
  void longArg(std::vector<std::string> &_argv);
};

struct Config {
  bool noSave = true;
  std::string http = "80";
  std::string https = "443";
  std::string key = "./key";
  std::string cert = "./cert";

  void load();
  void save();
};
