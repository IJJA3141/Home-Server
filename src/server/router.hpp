#pragma once

#include "reqres.hpp"
#include <functional>
#include <string>
#include <vector>

class Router
{
public:
  Router() {};

  void add(const Method _method, std::string _path,
           const std::function<Response(Request)> &_lambda);
  void add_error_handler(Request::Failure _err, const std::function<Response(Request)> &_lambda);
  Response respond(Request _req) const;
  Response handle_err(Request _req) const;

private:
  struct Route {
    const std::function<Response(Request)> *methods[method_size] = {};
    std::vector<std::string> path;
  };

  const std::function<Response(Request)> *error_handler_[Request::failure_size] = {};
  std::vector<Route> paths_;
};
