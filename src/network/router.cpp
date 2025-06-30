#include "router.hpp"
#include "../log.hpp"
#include "http.hpp"

Router::Router(Response _fallback) { this->error_handlers[Request::Error::NONE] = _fallback; }

void Router::add(Method _method, std::string_view _path, std::function<Response(Request)> _function)
{
  assert(_path[0] == '/', _path, "is invalid paths should start with '/'");
  _path.remove_prefix(1);

  Router::Route new_route;
  std::size_t pos;

  // create path
  while ((pos = _path.find("/")) != _path.npos)
  {
    new_route.path.push_back(std::string(_path.substr(0, pos)));
    _path.remove_prefix(pos);
  }

  new_route.path.push_back(std::string(_path));

  // check for existing path
  for (Route& route : this->routes_)
  {
    if (route.path == new_route.path)
    {
      check(Level::WARN, route.functions[_method].has_value(), "over vrite...");
      route.functions[_method] = _function;
      return;
    }
  }

  new_route.functions[_method] = _function;
  this->routes_.push_back(new_route);

  return;
}

void Router::add(Request::Error _error, Response _response)
{
  check(Level::WARN, this->error_handlers[_error].has_value(), "");
  this->error_handlers[_error] = _response;

  return;
}

Response Router::respond(Request _request) const
{
  if (!check(Level::WARN, _request.state == Request::Error::NONE, "failed", std::string(_request)))
    return this->handle_error(_request.state);

  for (const Route& route : this->routes_)
  {
    if (route.path == _request.cmd.url.path)
    {
      if (route.functions[_request.cmd.method].has_value())
        return route.functions[_request.cmd.method].value()(_request);

      return this->handle_error(Request::Error::I_METHOD);
    }
  }

  return this->handle_error(Request::Error::I_URL);
}

Response Router::handle_error(Request::Error _error) const
{
  if (check(Level::WARN, this->error_handlers[_error].has_value(), "no handler for", _error))
    return this->error_handlers[_error].value();

  return this->error_handlers[Request::Error::NONE].value();
}
