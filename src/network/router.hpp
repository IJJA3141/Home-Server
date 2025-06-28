#pragma once

#include "http.hpp"
#include <functional>
#include <string>

class Router
{
public:
  Response respond(Request _request) const;
  Response handle_error(Request::Error _error) const;

  void add(const Method _method, std::string_view _path, const std::function<Response(Request)> _λ);
  void set_error_handler(const Request::Error _e, const Response _response);

private:
  struct Route
  {
    std::function<Response(Request)>* λ[method_size];
    std::vector<std::string> path;
  };

  std::vector<Route> routes_;
  Response* error_handlers[Request::error_size - 2]; // -2 for NONE and CLOSED
};
