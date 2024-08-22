#pragma once

#include "parser.hpp"
#include "server.hpp"

#include <functional>
#include <string>

struct Path {
  std::vector<std::string> match;
  const std::function<std::string(Request _message)> *methods[8];

  Path(std::string _path);
};

class Router
{
public:
  Router(std::string _execPath);

  void add(const Method _method, std::string _path,
           const std::function<std::string(Request)> &_lambda);
  std::string respond(Request _message) const;
  std::string failed() const;
  std::string loadFile(std::string _path);
  std::string handleErr(const Request _message) const;
  std::string notInPath() const;

private:
  std::string execPath_;
  std::vector<Path> paths_;
};
