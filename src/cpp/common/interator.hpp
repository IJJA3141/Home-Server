#pragma once

#include <span>
#include <string_view>

class Iterator
{
public:
  Iterator(std::span<const char> _) : tail(std::string_view(_)) {}

  std::string_view tail;
  std::string_view head;

  /**
   *  @brief  Extracts the leading substring up to @a _delimiter.
   *  @param  _delimiter  Delimiter marking the end of the extracted substring.
   *  @return  True if the delimiter was found, false otherwise.
   *
   *  Searches tail for @a _delimiter. If found, the substring from the
   *  beginning of tail up to (but not including) @a _delimiter is extracted
   *  into head (@a _delimiter is removed from tail). If the delimiter is not
   *  found, tail remains unchanged.
   */
  bool next(const std::string_view _delimiter)
  {
    size_t pos = tail.find(_delimiter);
    if (pos == tail.npos) return false;

    head = tail.substr(0, pos);
    tail.remove_prefix(pos + _delimiter.size());

    return true;
  }

  /**
   *  @brief  Extracts the leading substring up to @a _delimiter.
   *  @param  _delimiter  Delimiter marking the end of the extracted substring.
   *  @return  True if the delimiter was found, false otherwise.
   *
   *  Searches tail for @a _delimiter. If found, the substring from the
   *  beginning of tail up to (but not including) @a _delimiter is extracted
   *  into head (@a _delimiter is removed from tail). If the delimiter is not
   *  found, tail remains unchanged.
   */
  bool next(const char _delimiter)
  {
    size_t pos = tail.find(_delimiter);
    if (pos == tail.npos) return false;

    head = tail.substr(0, pos);
    tail.remove_prefix(pos + 1);

    return true;
  }
};
