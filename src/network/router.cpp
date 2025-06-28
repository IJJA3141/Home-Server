#include "router.hpp"
#include "../log.hpp"
#include "http.hpp"

void Router::add(const Method _method, std::string_view _path, const std::function<Response(Request)> _λ)
{
  assert(_path[0] == '/', _path, "is invalid paths should start with '/'.");
  _path.remove_prefix(1);

  Router::Route new_route;
  std::size_t pos;

  while ((pos = _path.find("/")) != _path.npos)
  {
    new_route.path.push_back(std::string(_path.substr(0, pos)));
    _path.remove_prefix(pos);
  }

  new_route.path.push_back(std::string(_path));

  for (auto& route : this->routes_)
  {
    if (route.path == new_route.path)
    {
      if (route.λ[_method] != nullptr)
      {
        warn("over writing", method_to_string(_method), "at", route.path);
        delete route.λ[_method];
      }

      route.λ[_method] = new std::function<Response(Request)>(_λ);
      return;
    }
  }

  log("new route", method_to_string(_method), new_route.path);
  new_route.λ[_method] = new std::function<Response(Request)>(_λ);
  this->routes_.push_back(new_route);

  return;
}

void Router::set_error_handler(const Request::Error _e, const Response _response)
{
  if (this->error_handlers[_e] == nullptr)
  {
    warn("over writing", _e, "error handler");
    delete this->error_handlers[_e];
  }

  this->error_handlers[_e] = new Response(_response);
}

Response Router::respond(Request _request) const
{
  if (_request.state != Request::Error::NONE) return this->handle_error(_request.state);

  for (const auto& route : this->routes_)
  {
    if (route.path == _request.cmd.url.path)
    {
      if (route.λ[_request.cmd.method] == nullptr) return this->handle_error(Request::Error::I_METHOD);

      return (*route.λ[_request.cmd.method])(_request);
    }
  }

  return this->handle_error(Request::Error::NOT_FOUND);
}

Response Router::handle_error(Request::Error _e) const
{
  assert(this->error_handlers[_e] != nullptr, _e, "not handled");
  return *this->error_handlers[_e];
}
