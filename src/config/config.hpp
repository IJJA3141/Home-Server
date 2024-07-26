#pragma once

#include <functional>
#include <string>

struct Handler {
  enum Type { STRING, VOID, INT };
  std::function<void(std::string)> fn;
  const std::string key;
  const Type type;

  Handler(std::string _key, std::function<void(std::string)> _lambda);
  Handler(std::string _key, std::function<void(void)> _lambda);
  Handler(std::string _key, std::function<void(int)> _lambda);
};

struct Arg {
  std::string key;
  std::string *arg;

  ~Arg() { delete this->arg; };
};

void parse(const std::vector<Arg> &_args, const std::vector<Handler> _handlers);

struct Config {
  bool verbose = false;

  void load(std::string _path);
};
