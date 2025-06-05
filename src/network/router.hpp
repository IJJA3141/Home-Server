#pragma once

#include "http.hpp"

#include <functional>
#include <string>
#include <vector>

class Router {
public:
  void add(const http::Method _method, std::string _path, const std::function<http::Response(http::Request)> &&_lambda);
  void add(const http::Method _method, std::string _path, const std::function<http::Response(http::Request)> &_lambda);
  void add_error_handler(http::Request::Failure _err, const std::function<http::Response(http::Request)> &&_lambda);
  void add_error_handler(http::Request::Failure _err, const std::function<http::Response(http::Request)> &_lambda);

  http::Response respond(http::Request _req) const;
  http::Response handle_err(http::Request _req) const;

private:
  struct Route{
    const std::function<http::Response(http::Request)> *methods[http::METHOD_SIZE] = {};
    std::vector<std::string> path;

    Route(std::string _path);
  };

  const std::function<http::Response(http::Request)> *error_handler_[http::Request::FAILURE_SIZE] = {};
  std::vector<Route> routes_;
};
