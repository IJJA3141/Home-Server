#include "router.hpp"
#include "../log.hpp"

#include <fstream>
#include <unordered_map>

Path::Path(std::string _path) : match("")
{
  size_t start = _path.find("[");
  size_t end = _path.find("]");
  size_t index = 0;

  while (_path.size() > 0) {
    if (start != std::string::npos && end != std::string::npos && start < end) {
      for (size_t i = 1; i < start; i++)
        if (_path[i] == '/') index++;

      this->skip.push_back({index++, _path.substr(start + 1, end - start - 1)});
      this->match += _path.substr(0, start - 1);
      _path.erase(0, end + 1);
      PRINT(_path);
    } else {
      this->match += _path;

      break;
    }

    start = _path.find("[");
    end = _path.find("]");
  }

  return;
}

Router::Router(std::string _execPath)
{
  this->execPath_ = _execPath.substr(0, _execPath.rfind("/"));
  return;
};

void Router::add(Method _method, std::string _path, std::function<std::string(void)> *_lambda,
                 bool _needSSL)
{
  // path sanitization
  if (_path[_path.size() - 1] == '/') _path = _path.substr(0, _path.size() - 1);

  Path path(_path);
  if (_needSSL) {
    path.SSLMethods[_method] = _lambda;
  } else {
    path.methods[_method] = _lambda;
  }

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

std::string Router::respond(const Message _message, Client::Type _clientType) const
{
  std::string target = _message.cmd.path;
  std::unordered_map<std::string, std::string> map;

  size_t index = 1;

  for (const Path &path : this->paths_) {
    for (const std::pair<int, std::string> &pair : path.skip) {
      for (int i = 0; i <= pair.first; i++) {
        index += target.find("/", index + 1);
      }

      size_t end = target.find("/", index + 1);
      map[pair.second] = target.substr(index, end - index);
      target.erase(index, end - index);
    }
  }

  return "";
};
