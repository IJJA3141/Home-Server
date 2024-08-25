#include "router.hpp"
#include "../log.hpp"

void Router::add(const Method _method, std::string _path,
                 const std::function<Response(Request)> &_lambda)
{
  for (auto &route : this->paths_) {
    std::string comp = "";
    for (size_t i = 0; i < route.path.size(); i++)
      comp += route.path[i];

    if (comp == _path) {
      if (route.methods[_method] != nullptr) WARN(_path << " has been overwriten");
      route.methods[_method] = &_lambda;
    }
  }

  Route route;

  if (_path.size() == 0 || _path.find('#') != std::string::npos ||
      _path.find('?') != std::string::npos) {
    ERR("realy ?");
    exit(1);
  }

  if (_path[0] != '/') {
    ERR(_path << "should start with a / this route will be ignored");
    exit(1);
  }

  if (_path.size() == 1) {
    route.path = {"/"};
  } else {
    while (_path.size() > 2 && _path[_path.size() - 1] == '/')
      _path.pop_back();

    size_t start = 0;
    size_t end = 0;
    while ((end = _path.find('/', end + 1)) != std::string::npos) {
      if (start == end + 1) {
        start = end;
        continue;
      }

      route.path.push_back(_path.substr(start, end - start));
      start = end;
    }
    route.path.push_back(_path.substr(start, end - start));
  }

  LOG("new route added: " << _path);

  if (route.methods[_method] != nullptr) WARN(_path << " has been overwriten");
  route.methods[_method] = &_lambda;
  this->paths_.push_back(route);

  return;
}

void Router::add_error_handler(Request::Failure _err, std::function<Response(Request)> &_lambda)
{
  if (this->error_handler_[_err] != nullptr) WARN("an error handler has been overwriten");
  this->error_handler_[_err] = &_lambda;

  return;
}

Response Router::respond(Request _req) const
{
  if (_req.failure != Request::Failure::NONE) return this->handle_err(_req);

  for (const Router::Route &route : this->paths_) {
    if (route.path.size() != _req.cmd.path.size()) continue;
    _req.url_args.clear();

    for (size_t i = 0; i < route.path.size(); i++) {
      if (route.path[i][1] == '[') {
        size_t end = route.path[i].find(']');

        if (end == std::string::npos) {
          ERR("there is an unclosed bracket in a route");
          WARN("a route has been ignored due to an error in it's path");
          goto next;
        }

        _req.url_args[route.path[i].substr(2, end - 2)] = _req.cmd.path[i].substr(1);
        continue;
      }

      if (route.path[i] != _req.cmd.path[i]) goto next;
    }

    if (route.methods[_req.cmd.method] != nullptr) return (*route.methods[_req.cmd.method])(_req);

    _req.failure = Request::Failure::UNAUTHORIZEDMETHOD;
    return this->handle_err(_req);

  next:;
  }

  _req.failure = Request::Failure::WRONGPATH;
  return this->handle_err(_req);
}

Response Router::handle_err(Request _req) const { return {}; };
