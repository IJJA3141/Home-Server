#include "router.hpp"
#include "../log.hpp"

#include <fstream>

Path::Path(std::string _path)
{
  if (_path.size() == 0) {
    ERR("[empty] is not a valid path");
    exit(1);
  }

  if (_path[0] != '/') {
    ERR("a path should start with /");
    exit(1);
  }

  // path sanitization
  while (_path.size() >= 2 && _path[_path.size() - 1] == '/') {
    _path = _path.substr(0, _path.size() - 1);
    WARN("you should't have a tailing / in your path");
  }

  if (_path.size() == 1) {
    this->match = {"/"};
    return;
  }

  size_t start = 0;
  size_t end = 0;
  while ((end = _path.find('/', end + 1)) != std::string::npos) {
    if (start == end + 1) {
      start = end;
      continue;
    }

    this->match.push_back(_path.substr(start, end - start));
    start = end;
  }
  this->match.push_back(_path.substr(start, end - start));

  return;
}

Router::Router(std::string _execPath)
{
  this->execPath_ = _execPath.substr(0, _execPath.rfind("/"));
  return;
};

void Router::add(Method _method, std::string _path,
                 const std::function<std::string(Request)> &_lambda)
{
  Path path(_path);
  path.methods[_method] = &_lambda;

  this->paths_.push_back(path);

  return;
};

std::string Router::loadFile(std::string _path)
{
  std::ifstream steam((this->execPath_ + _path));
  if (!steam.is_open()) VERBERR("file opening failed");

  std::string str((std::istreambuf_iterator<char>(steam)), std::istreambuf_iterator<char>());
  PRINT(str);
  steam.close();

  return str;
}

std::string Router::respond(Request _message, Client::Type _clientType) const
{
  if (_message.failure != Request::Failure::NONE) return this->handleErr(_message);

  for (const Path &path : this->paths_) {
    if (path.match.size() != _message.cmd.path.size()) continue;
    _message.urlParam.clear();

    for (size_t i = 0; i < path.match.size(); i++) {
      if (path.match[i][1] == '[') {
        size_t end = path.match[i].find(']');

        if (end == std::string::npos) {
          WARN("bracket not closed in path");
          goto next;
        } else {
          _message.urlParam[path.match[i].substr(2, end - 2)] = _message.cmd.path[i].substr(1);
        }

        continue;
      }

      if (path.match[i] != _message.cmd.path[i]) goto next;
    }

    if (path.methods[_message.cmd.method] != nullptr) {
      return (*path.methods[_message.cmd.method])(_message);
    } else
      return this->notInPath();

  next:;
  }

  return this->notInPath();
};

std::string Router::handleErr(const Request _message) const { return ""; }
std::string Router::notInPath() const { return ""; };
