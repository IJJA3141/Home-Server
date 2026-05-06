#include "router.hpp"
#include <cassert>

using Request = ISegment::Request;
using Response = ISegment::Response;
using Value = ISegment::Value;
using Table = ISegment::Table;

inline size_t find(const Table& table, std::string_view key)
{
  size_t i = 0;
  while (table[i].first != key && ++i < table.size())
    ;
  return i;
}

Response EmptySegment::resolve(Iterator& _path, Request& _request)
{
  assert(_path.tail.empty() || _path.tail.back() != '/');

  size_t i = find(this->table_, _path.head);

  if (!_path.tail.empty() && _path.tail[0] != '?')
  { // not the end segment
    if (i < this->table_.size())
    { // next segment is already registered reple down
      return this->table_[i].second->resolve(_path, _request);
    }
    else
    { // no next segment add an empty one for now
      return {};
    }
  }
  else
  {
    if (i < this->table_.size())
    { // segment is already registered
      return this->table_[i].second
    }
    else
    { // add new segment
      this->table_.emplace_back(_path.head, std::move(_segment));
    }
  }
}

void EmptySegment::insert(Iterator& _path, std::unique_ptr<ISegment>& _segment)
{
  assert(_path.tail.empty() || _path.tail.back() != '/');
  size_t i = find(this->table_, _path.head);

  if (_path.next('/'))
  { // not the end segment
    if (i < this->table_.size())
    { // next segment is already registered reple down
      this->table_[i].second->insert(_path, _segment);
    }
    else
    { // no next segment add an empty one for now
      this->table_.emplace_back(_path.head, std::make_unique<EmptySegment>(_path, _segment));
    }
  }
  else
  { // path is end segment
    if (i < this->table_.size())
    { // segment is already registered
      _segment->set_table(this->table_[i].second->get_table());
      std::swap(this->table_[i].second, _segment);
    }
    else
    { // add new segment
      this->table_.emplace_back(_path.head, std::move(_segment));
    }
  }
}

Table EmptySegment::get_table() { return std::move(this->table_); }
void EmptySegment::set_table(Table _table) { std::swap(this->table_, _table); }
