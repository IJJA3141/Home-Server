#include "../logger/logger.hpp"
#include "../protocol/http/iterator.hpp"
#include "routing.hpp"
#include <algorithm>
#include <string_view>

constexpr std::array invalid_chars{':', '?', '#', '@', '!', '!', '$', '&', '\'', '(', ')', '+', ',',
                                   ';', '=', '<', '>', '"', '{', '}', '|', '\\', '^', '`', ' ', '%'};

void assert_braces(std::string_view segment, const std::string_view _path)
{
  auto log = Logger::New();

  if (segment.empty()) return;
  if (segment.front() == '[')
  {
    if (segment.back() != ']')
    {
      log.crit("malformed path (unclosed brackets): {}", _path);
      throw;
    }

    std::string_view view = segment.substr(1, segment.size() - 2);
    if (view.contains('[') || view.contains(']'))
    {
      log.crit("malformed path (brackets in brackets): {}", _path);
      throw;
    }
  }
  else if (segment.contains(']'))
  {
    log.crit("malformed path (unopend brackets): {}", _path);
    throw;
  }
}

void assert_valid(const std::string_view _path)
{
  auto log = Logger::New();
  std::string_view path = _path;

  if (std::any_of(invalid_chars.begin(), invalid_chars.end(), [=](char _) { return path.contains(_); }))
  {
    log.crit("path contains one of {} invalid char {}", invalid_chars, _path);
    throw "";
  }

  if (!path.starts_with('/'))
  {
    log.crit("path must start with '/': {}", _path);
    throw "";
  }

  if (path.contains('*'))
  {
    if (!path.ends_with("/*"))
    {
      log.crit("wildcard must be at the end (/*): {}", _path);
      throw "";
    }

    path.remove_suffix(1);
    if (path.contains('*'))
    {
      log.crit("multiple wildcards are not supported: {}", _path);
      throw "";
    }
  }

  Iterator it(_path);
  while (it.next('/'))
    assert_braces(it.head, _path);

  if (it.tail.contains('[') || it.tail.contains(']'))
  {
    log.crit("last segment cannot be parametric: {}", _path);
    throw "";
  }
}

std::string_view regularise_path(std::string_view _path)
{
  _path.remove_prefix(1);
  if (_path.ends_with('/')) _path.remove_suffix(1);
  return _path;
}
