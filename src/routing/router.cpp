#include "../logger/logger.hpp"
#include "../protocol/http/iterator.hpp"
#include "routing.hpp"
#include <memory>
#include <string_view>

constexpr Segment::ParentingType parent_type(std::string_view child_route)
{
  if (child_route.starts_with('[')) return Segment::ParentingType::PARAMETRIC;
  return Segment::ParentingType::STATIC;
}

Segment* Router::get(std::string_view _path)
{
  assert_valid(_path);

  Logger log = Logger::New();
  Iterator path(regularise_path(_path));
  Segment* target_parent = this->root.get();

  while (path.next('/'))
  {
    switch (target_parent->type)
    {
    case Segment::ParentingType::WILDCARD:
      throw;

    case Segment::ParentingType::PARAMETRIC:
      if (!path.head.starts_with('[')) throw;
      if (path.head.subview(1, path.head.size() - 2) != target_parent->meta) throw;
      if (!path.next('/')) goto BREAK;
      break;

    case Segment::ParentingType::STATIC:
      if (path.head.starts_with('[')) throw;
      break;

    case Segment::ParentingType::CHILDLESS:
      if (path.head.starts_with('['))
      {
        target_parent->meta = path.head.subview(1, path.head.size() - 2);
        target_parent->type = Segment::ParentingType::PARAMETRIC;
        if (!path.next('/')) goto BREAK;
        break;
      }

      target_parent->type = Segment::ParentingType::STATIC;
      break;
    }

    target_parent = target_parent->get_or_create(path.head);
  }
BREAK:

  if (path.tail.starts_with('[')) throw;

  switch (target_parent->type)
  {
  case Segment::ParentingType::STATIC:
    if (path.tail.ends_with('*')) throw;
    break;

  case Segment::ParentingType::PARAMETRIC:
    break;

  case Segment::ParentingType::WILDCARD:
    if (!path.tail.ends_with('*')) throw;
    break;

  case Segment::ParentingType::CHILDLESS:
    if (path.tail.ends_with('*')) target_parent->type = Segment::ParentingType::WILDCARD;
    else target_parent->type = Segment::ParentingType::STATIC;
    break;
  }

  return target_parent->get_or_create(path.tail);
}

void Router::add(Method _method, std::string_view _path, Handler _handler)
{
  auto& [_, handler] = this->get(_path)->handlers[_method];
  if (handler) throw;
  handler = _handler;
}

void Router::add(Method _method, std::string_view _path, Middleware _middleware, Handler _handler)
{
  auto& [middleware, handler] = this->get(_path)->handlers[_method];
  if (handler) throw;
  middleware = _middleware;
  handler = _handler;
}

// void Router::add(std::string_view _prefix, Router&& _subtree)
// {
//   Segment* parent = this->get(_prefix);
//   parent->table.reserve(_subtree.root->table.size());
//
//   for (auto& [route, segment] : _subtree.root->table)
//     parent->table.emplace_back(route, std::move(segment));
// }
