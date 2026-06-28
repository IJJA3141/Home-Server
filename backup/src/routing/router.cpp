#include "../config.hpp"
#include "../logger/logger.hpp"
#include "../utils/exception.hpp"
#include "../utils/iterator.hpp"
#include "routing.hpp"
#include <string_view>

Segment* Router::get(std::string_view _route)
{
  Iterator route(_route);
  Segment* target_parent = this->root.get();

  while (route.next('/'))
  {
    switch (target_parent->type)
    {
    case Segment::ParentingType::WILDCARD:
      throw configuration_error("segment under wildcard /{}", _route);

    case Segment::ParentingType::PARAMETRIC:
      if (!route.head.starts_with('['))
        throw configuration_error("missing [{}] parameter /{}", target_parent->meta, _route);

      if (route.head.subview(1, route.head.size() - 2) != target_parent->meta)
        throw configuration_error("wrong parameter expected /{} got {} {}", target_parent->meta,
                                  route.head.subview(1, route.head.size() - 2), _route);

      if (route.tail.starts_with('[')) throw configuration_error("multiple paramter in a row {}", _route);

      if (!route.next('/')) goto BREAK;
      break;

    case Segment::ParentingType::STATIC:
      if (route.head.starts_with('[')) throw configuration_error("unexpected parameter /{}", _route);
      break;

    case Segment::ParentingType::CHILDLESS:
      if (route.head.starts_with('['))
      {
        target_parent->meta = route.head.subview(1, route.head.size() - 2);
        target_parent->type = Segment::ParentingType::PARAMETRIC;
        if (!route.next('/')) goto BREAK;
        break;
      }

      target_parent->type = Segment::ParentingType::STATIC;
      break;
    }

    target_parent = target_parent->get_or_create(route.head);
  }
BREAK:

  if (route.tail.starts_with('[')) throw configuration_error("unexpected parameter /{}", _route);

  switch (target_parent->type)
  {
  case Segment::ParentingType::STATIC:
    if (route.tail.ends_with('*')) throw configuration_error("wildcard not allowed on static /{}", _route);
    break;

  case Segment::ParentingType::PARAMETRIC:
    if (route.tail.ends_with('*')) throw configuration_error("wildcard not allowed on parametric /{}", _route);
    break;

  case Segment::ParentingType::WILDCARD:
    if (!route.tail.ends_with('*')) throw configuration_error("non-wildcard under wildcard /{}", _route);
    break;

  case Segment::ParentingType::CHILDLESS:
    if (route.tail.ends_with('*')) target_parent->type = Segment::ParentingType::WILDCARD;
    else target_parent->type = Segment::ParentingType::STATIC;
    break;
  }

  return target_parent->get_or_create(route.tail);
}

void Router::add(Method _method, Route _route, Handler _handler)
{
  auto& [_, handler] = this->get(_route)->handlers[_method];
  if (handler)
    throw configuration_error("duplicate handler for {} /{}", protocol::HTTP::method_to_string(_method),
                              (std::string)_route);

  handler = _handler;
}

void Router::add(Method _method, Route _route, Middleware _middleware, Handler _handler)
{
  auto& [middleware, handler] = this->get(_route)->handlers[_method];
  if (handler || middleware)
    throw configuration_error("duplicate handler/middleware for {} /{}", protocol::HTTP::method_to_string(_method),
                              (std::string)_route);

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

const Segment* Router::get(std::string_view _route, Request& _request) const
{
  Iterator route(_route);
  const Segment* target_parent = this->root.get();
  if (target_parent == nullptr) return nullptr;

  while (route.next('/'))
  {
    switch (target_parent->type)
    {
    case Segment::ParentingType::WILDCARD:
      _request.headers[HTTP_WILDCARD] = std::string(route.head) + "/";
      goto BREAK;

    case Segment::ParentingType::PARAMETRIC:
      _request.headers[HTTP_PREFIX + target_parent->meta] = route.head;
      if (!route.next('/')) return target_parent->find(route.tail);
      break;

    case Segment::ParentingType::STATIC:
      break;

    case Segment::ParentingType::CHILDLESS:
      return nullptr;
    }

    target_parent = target_parent->find(route.head);
    if (target_parent == nullptr) return nullptr;
  }
BREAK:

  switch (target_parent->type)
  {
  case Segment::ParentingType::CHILDLESS:
    return nullptr;

  case Segment::ParentingType::STATIC:
    return target_parent->find(route.tail);

  case Segment::ParentingType::PARAMETRIC:
    _request.headers[HTTP_PREFIX + target_parent->meta] = route.tail;
    return target_parent->find("");

  case Segment::ParentingType::WILDCARD:
    _request.headers[HTTP_WILDCARD] += route.tail;
    return target_parent->find("*");
  }

  return target_parent;
}

Router::Response Router::handle(Request& _request) const
{
  std::string_view route = _request.path;
  if (route.contains('?')) route.remove_suffix(route.size() - route.find('?'));

  Logger::debug("(Router) resolving {}", route);

  if (Route::invalid(route) || route.contains('[') || route.contains(']') || !route.starts_with('/'))
    return this->err(protocol::HTTP::standard_response(400));

  route.remove_prefix(1);
  if (route.ends_with('/')) route.remove_suffix(1);

  const Segment* seg = this->get(route, _request);
  if (seg == nullptr)
  {
    Logger::warn("route {} not found", route);
    return this->err(protocol::HTTP::standard_response(404));
  }

  Logger::debug("route {} found", route);
  const auto& [middleware, handler] = seg->handlers[_request.method];
  // if (!handler) return this->err(protocol::HTTP::standard_response(405));
  if (!handler)
  {
    Logger::debug("request resolved to unsuported method");
    return protocol::HTTP::standard_response(500);
  }

  if (middleware)
  {
    Logger::debug("middleware -> handler");
    return err(middleware(_request, handler));
  }
  else
  {
    Logger::debug("handler");
    return err(handler(_request));
  }
}

Router::Response Router::err(Response&& res) const { return res; }
