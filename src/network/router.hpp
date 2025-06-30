#pragma once

#include "http.hpp"

#include <functional>
#include <optional>

class Router
{
public:
  Router(Response _fallback);

  void add(Method _method, std::string_view _path, std::function<Response(Request)> _function);
  void add(Request::Error _error, Response _response);

  Response respond(Request _request) const;
  Response handle_error(Request::Error _error) const;

private:
  struct Route
  {
    std::optional<std::function<Response(Request)>> functions[method_size];
    std::vector<std::string> path;
  };

  std::optional<Response> error_handlers[Request::error_size]; // -1 for NONE
  std::vector<Route> routes_;
};
