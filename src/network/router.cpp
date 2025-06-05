#include "router.hpp"
#include "../log.hpp"

#include <string>

Router::Route::Route(std::string _path) {
  if (_path.size() == 0 || _path.find('#') != std::string::npos || _path.find('?') != std::string::npos || _path[0] != '/') {
    ERR("realy ?");
    exit(1);
  }

  if (_path.size() == 1) {
    this->path = {"/"};
    return;
  }

  while (_path.size() > 2 && _path[_path.size() - 1] == '/') _path.pop_back();

  size_t start = 0;
  size_t end = 0;
  while ((end = _path.find('/', end + 1)) != std::string::npos) {
    if (start == end + 1) {
      start = end;
      continue;
    }

    this->path.push_back(_path.substr(start, end - start));
    start = end;
  }

  this->path.push_back(_path.substr(start, end - start));
}

void Router::add(const http::Method _method, std::string _path, const std::function<http::Response(http::Request)> &&_lambda) {
  this->add(_method, _path, *new std::function<http::Response(http::Request)>(_lambda));
  return;
}

void Router::add(const http::Method _method, std::string _path, const std::function<http::Response(http::Request)> &_lambda) {
  // search if route exits
  for (auto &route : this->routes_) {
    std::string path = "";

    for (size_t i = 0; i < route.path.size(); i++) path += route.path[i];

    if (path == _path) {
      if (route.methods[_method] != nullptr) WARN(_path << " has been overwriten");
      route.methods[_method] = &_lambda;
      return;
    }
  }

  // add new route
  Route route(_path);
  route.methods[_method] = &_lambda;

  this->routes_.push_back(route);
  LOG("new route added: " << _path);
  return;
}
