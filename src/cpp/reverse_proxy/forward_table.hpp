#pragma once

#include "../common/concepts.hpp"
#include <set>
#include <string_view>

class ForwardingTable
{
public:
  concepts::Writable auto& resolve(std::string_view host);
  void add(std::string key, concepts::Writable auto& value);
};
