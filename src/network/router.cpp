#include "router.hpp"
#include "../log.hpp"
#include "http.hpp"

#define WAR_OVERRIDE_ERR   ""
#define WAR_OVERRIDE_ROUTE ""
#define WAR_RES_FAILED     "the following request failed\n", std::string(_request)
#define WAR_NO_ERR         ""

Router::Router(http::Response _fallback) { this->error_handlers_[http::Error::NONE] = _fallback; }

void Router::add(const http::Method _method, const std::string_view _path,
                 const std::function<http::Response(http::Request, std::string)> _function)
{
  this->add(_method, std::make_shared<SRoute>(std::string(_path), _function));
  return;
}

void Router::add(const http::Method _method, const std::string_view _path,
                 const std::function<http::Response(http::Request)> _function)
{
  this->add(_method, std::make_shared<Route>(std::string(_path), _function));
  return;
}

void Router::add(const http::Method _method, const std::string_view _path, const http::Response _response)
{
  this->add(_method, std::make_shared<CRoute>(std::string(_path), _response));
  return;
}

void Router::add(const http::Method _method, const std::shared_ptr<IRoute> _route)
{
  for (std::shared_ptr<IRoute>& route : this->routes_[_method])
  {
    if ((*route) == (*_route))
    {
      warn(WAR_OVERRIDE_ROUTE);
      route = _route;
      return;
    }
  }

  this->routes_[_method].push_back(_route);
  return;
}

void Router::add(const http::Error _error, const http::Response _response)
{
  check(Level::WARN, !this->error_handlers_[_error].has_value(), WAR_OVERRIDE_ERR);
  this->error_handlers_[_error] = _response;
  return;
}

http::Response Router::respond(const http::Request _request) const
{
  if (_request.state != http::Error::NONE)
  {
    warn(WAR_RES_FAILED);
    return this->handle_error(_request.state);
  }

  for (const auto& route : this->routes_[_request.cmd.method])
  {
    if (route->match(_request.cmd.url.path))
    {
      return route->invoke(_request);
    }
  }

  for (int i = 1; i < http::method_size; ++i)
  {
    for (const auto& route : this->routes_[(_request.cmd.method + i) % http::method_size])
    {
      if (route->match(_request.cmd.url.path))
      {
        return this->handle_error(http::Error::I_METHOD);
      }
    }
  }

  warn(_request.cmd.url.path, "not found.");
  return this->handle_error(http::Error::I_URL);
}

http::Response Router::handle_error(const http::Error _error) const
{
  if (this->error_handlers_[_error].has_value()) return this->error_handlers_[_error].value();

  warn(WAR_NO_ERR);
  return this->error_handlers_[http::Error::NONE].value();
}
