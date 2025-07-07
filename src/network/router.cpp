#include "router.hpp"
#include "../log.hpp"
#include "http.hpp"

Router::Router(http::Response _fallback) { this->error_handlers[http::Error::NONE] = _fallback; }

void Router::add(http::Method _method, std::string_view _path,
                 std::function<http::Response(http::Request)> _function)
{
  assert(_path[0] == '/', _path, "is invalid paths should start with '/'");

  // check for existing path
  for (Route& route : this->routes_)
  {
    if (route.path == _path)
    {
      check(Level::WARN, route.functions[_method].has_value(), "over vrite...");
      route.functions[_method] = _function;
      return;
    }
  }

  Router::Route new_route{std::string(_path)};
  new_route.functions[_method] = _function;
  this->routes_.push_back(new_route);

  return;
}

void Router::add(http::Error _error, http::Response _response)
{
  check(Level::WARN, this->error_handlers[_error].has_value(), "");
  this->error_handlers[_error] = _response;

  return;
}

http::Response Router::respond(http::Request _request) const
{
  if (!check(Level::WARN, _request.state == http::Error::NONE, "failed", std::string(_request)))
    return this->handle_error(_request.state);

  for (const Route& route : this->routes_)
  {
    if (route.path == _request.cmd.url.path)
    {
      if (route.functions[_request.cmd.method].has_value())
        return route.functions[_request.cmd.method].value()(_request);

      return this->handle_error(http::Error::I_METHOD);
    }
  }

  return this->handle_error(http::Error::I_URL);
}

http::Response Router::handle_error(http::Error _error) const
{
  if (check(Level::WARN, this->error_handlers[_error].has_value(), "no handler for", _error))
    return this->error_handlers[_error].value();

  return this->error_handlers[http::Error::NONE].value();
}
