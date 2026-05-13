#include "../logger/logger.hpp"
#include "../utils/exception.hpp"
#include "../utils/iterator.hpp"
#include "routing.hpp"
#include <string_view>

Segment* Router::get(std::string_view _path)
{
  Logger log = Logger::New();
  Iterator path(_path);
  Segment* target_parent = this->root.get();

  while (path.next('/'))
  {
    switch (target_parent->type)
    {
    case Segment::ParentingType::WILDCARD:
      throw configuration_error("segment under wildcard {}", _path);

    case Segment::ParentingType::PARAMETRIC:
      if (!path.head.starts_with('['))
        throw configuration_error("missing [{}] parameter {}", target_parent->meta, _path);

      if (path.head.subview(1, path.head.size() - 2) != target_parent->meta)
        throw configuration_error("wrong parameter expected {} got {} {}", target_parent->meta,
                                  path.head.subview(1, path.head.size() - 2), _path);

      if (!path.next('/')) goto BREAK;
      break;

    case Segment::ParentingType::STATIC:
      if (path.head.starts_with('[')) throw configuration_error("unexpected parameter {}", _path);
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

  if (path.tail.starts_with('[')) throw configuration_error("unexpected parameter {}", _path);

  switch (target_parent->type)
  {
  case Segment::ParentingType::STATIC:
    if (path.tail.ends_with('*')) throw configuration_error("wildcard not allowed on static {}", _path);
    break;

  case Segment::ParentingType::PARAMETRIC:
    if (path.tail.ends_with('*')) throw configuration_error("wildcard not allowed on parametric {}", _path);
    break;

  case Segment::ParentingType::WILDCARD:
    if (!path.tail.ends_with('*')) throw configuration_error("non-wildcard under wildcard {}", _path);
    ;
    break;

  case Segment::ParentingType::CHILDLESS:
    if (path.tail.ends_with('*')) target_parent->type = Segment::ParentingType::WILDCARD;
    else target_parent->type = Segment::ParentingType::STATIC;
    break;
  }

  return target_parent->get_or_create(path.tail);
}

void Router::add(Method _method, Route _path, Handler _handler)
{
  auto& [_, handler] = this->get(_path)->handlers[_method];
  if (handler)
    throw configuration_error("duplicate handler for {} {}", protocol::HTTP::method_to_string(_method),
                              (std::string)_path);

  handler = _handler;
}

void Router::add(Method _method, Route _path, Middleware _middleware, Handler _handler)
{
  auto& [middleware, handler] = this->get(_path)->handlers[_method];
  if (handler || middleware)
    throw configuration_error("duplicate handler/middleware for {} {}", protocol::HTTP::method_to_string(_method),
                              (std::string)_path);

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
