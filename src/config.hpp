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

void parse(int _argc, char *_argv[], const std::vector<Handler> _handlers);
