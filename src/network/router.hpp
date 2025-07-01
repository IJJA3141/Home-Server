#pragma once

#include "http.hpp"

#include <functional>
#include <optional>

class Router
{
public:
  Router(http::Response _fallback);

  void add(http::Method _method, std::string_view _path, std::function<http::Response(http::Request)> _function);
  void add(http::Error _error, http::Response _response);

  http::Response respond(http::Request _request) const;
  http::Response handle_error(http::Error _error) const;

private:
  struct Route
  {
    std::string path;
    std::optional<std::function<http::Response(http::Request)>> functions[http::method_size];
  };

  std::optional<http::Response> error_handlers[http::error_size]; // -1 for NONE
  std::vector<Route> routes_;
};
