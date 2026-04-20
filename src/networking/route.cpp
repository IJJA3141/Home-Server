#include "router.hpp"

// x | !was_dash |  !is_dash |
// --|-----------|-----------|
// 1 |     1     |     1     |
// 1 |     1     |     0     |
// 1 |     0     |     1     |
// 0 |     0     |     0     |

// interface
IRoute::IRoute(const std::string& _path) : path_(_path)
{
  assert(!_path.empty(), "route path cannot be an empty string");
  assert(_path[0] == '/', "route path should start with '/'", _path);

  bool wasnt_dash = true;
  for (int i = 0; i < _path.size(); ++i)
    assert(wasnt_dash || (wasnt_dash = _path[i] != '/'), "malformed path", _path);

  return;
}

// route
Route::Route(const std::string& _path, const std::function<http::Response(http::Request)> _function)
    : IRoute(_path), function_(_function)
{
  assert(_path[_path.size() - 1] == '/', "route path should end with '/'", _path);
  return;
}

// static route
SRoute::SRoute(const std::string& _path, const std::function<http::Response(http::Request, std::string)> _function)
    : IRoute(_path), function_(_function)
{
  assert(_path[_path.size() - 1] == '/', "static route path should end with '/'");
  return;
}

bool SRoute::match(std::string_view _path) const
{
  if (_path.size() <= this->path_.size()) return false;

  _path.remove_suffix(_path.size() - this->path_.size());
  return this->path_ == _path;
}

http::Response SRoute::invoke(const http::Request _request) const
{
  return this->function_(_request, _request.cmd.url.path.substr(this->path_.size()));
}

// constant route
CRoute::CRoute(const std::string& _path, http::Response _response) : IRoute(_path), response_(_response)
{
  assert(_path[_path.size() - 1] == '/', "route path should end with '/'", _path);
  return;
}
