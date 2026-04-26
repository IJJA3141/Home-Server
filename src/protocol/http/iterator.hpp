#pragma once

#include <span>
#include <string_view>

class Iterator
{
public:
  std::string_view model;

  Iterator(const std::string_view _model) : model(_model) {};
  bool operator>>(std::string_view& _view)
  {
    std::size_t pos;
    if ((pos = this->model.find("\r\n")) == this->model.npos || pos == 0) return false;

    _view = this->model.substr(0, pos);
    this->model.remove_prefix(pos + 2);

    return true;
  };
};

class t
{
public:
  t(std::span<const char> _) : head(std::string_view(_)) {}

  std::string_view head;
  std::string_view tail;

  bool next(const std::string_view _delimiter)
  {
    size_t pos = head.find(_delimiter);
    if (pos == head.npos) return false;

    tail = head.substr(0, pos);
    head.remove_prefix(pos + _delimiter.size());

    return true;
  }

  bool next(const char _delimiter)
  {
    size_t pos = head.find(_delimiter);
    if (pos == head.npos || pos == 0) return false;

    tail = head.substr(0, pos);
    head.remove_prefix(pos + 1);

    return true;
  }
};
