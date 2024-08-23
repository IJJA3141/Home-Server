#pragma once

#include "client.hpp"
#include <functional>
#include <string>
#include <vector>

class Router
{
public:
  Router(const Method _method, const std::string _path,
         const std::function<Response(Request)> &_lambda);

  Response respond(Request _req) const;
  Response handle_err(Request _req) const;
  void add_error_handler(
      Request::Failure _err,
      std::function<Response(Request, std::map<std::string, std::string> &)> &_lambda);

private:
  struct Route {
    const std::function<Response(Request)> *methods[method_size];
    std::vector<std::string> path;

    Route(const std::string _path);
  };

  std::function<Response(Request, std::map<std::string, std::string> &)>
      *error_handler_[Request::failure_size];
  std::vector<Route> paths_;
};
