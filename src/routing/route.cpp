#include "../utils/exception.hpp"
#include "../utils/iterator.hpp"
#include "routing.hpp"
#include <algorithm>
#include <string_view>

constexpr std::array invalid_chars{':', '?', '#', '@', '!', '$', '&', '\'', '(',  ')', '+', ',',
                                   ';', '=', '<', '>', '"', '{', '}', '|',  '\\', '^', '`', ' '};

bool Route::invalid(std::string_view _route)
{
  return std::any_of(invalid_chars.begin(), invalid_chars.end(), [=](char _) { return _route.contains(_); });
}

void assert_braces(std::string_view _segment, const std::string_view _route)
{
  if (_segment.empty()) return;
  if (_segment.front() == '[')
  {
    std::string_view view = _segment.substr(1, _segment.size() - 2);
    if (_segment.back() != ']') throw malformed_route("unclosed brackets {}", _route);
    if (view.contains('[') || view.contains(']')) throw malformed_route("nested brackets {}", _route);
  }
  else if (_segment.contains(']')) throw malformed_route("unopend brackets {}", _route);
}

Route::Route(const std::string& _route)
{
  std::string_view route = _route;

  if (invalid(route)) throw malformed_route("one of {} invalid char {}", invalid_chars, _route);

  if (!route.starts_with('/')) throw malformed_route("route must start with '/' {}", _route);

  if (route.contains('*'))
  {
    if (!route.ends_with("/*")) throw malformed_route("wildcard must be at the end (/*) {}", _route);

    route.remove_suffix(1);
    if (route.contains('*')) throw malformed_route("multiple wildcards are not supported {}", _route);
  }

  Iterator it(_route);
  while (it.next('/'))
    assert_braces(it.head, _route);

  if (it.tail.contains('[') || it.tail.contains(']'))
    throw malformed_route("last segment cannot be parametric {}", _route);

  this->route = _route.substr(1, _route.size() - (_route.ends_with('/') && !_route.ends_with("]/") ? 2 : 0));
}
