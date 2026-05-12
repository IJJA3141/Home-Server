#include "routing.hpp"
#include <utility>

Segment* Segment::find(std::string_view _route)
{
  for (const auto& [route, segment] : this->table)
    if (route == _route) return segment.get();

  return nullptr;
}

Segment* Segment::get_or_create(std::string_view _route)
{
  Segment* target = this->find(_route);
  if (target) return target;
  else return this->table.emplace_back(_route, std::make_unique<Segment>()).second.get();
}

void Segment::sprint(std::string _parent, std::string& _out) const
{
  for (size_t i = 0; i < this->handlers.size(); ++i)
    if (this->handlers[i].second)
      _out += protocol::HTTP::method_to_string(protocol::HTTP::Method(i)) + " " + _parent +
              (this->handlers[i].first ? " +\n" : "\n");

  for (const auto& [route, segment] : this->table)
    switch (this->type)
    {
    case ParentingType::CHILDLESS:
      std::unreachable();

    case ParentingType::STATIC:
      segment->sprint(_parent + "/" + route, _out);
      break;

    case ParentingType::PARAMETRIC:
      segment->sprint(_parent + "/\x1b[33m[" + this->meta + "]\x1b[39m/" + route, _out);
      break;

    case ParentingType::WILDCARD:
      segment->sprint(_parent + "/" + "\x1b[31m" + route + "\x1b[39m", _out);
      break;
    }
}
