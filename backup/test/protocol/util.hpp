#pragma once

#include "../../src/protocol/http/http.hpp"
#include "../../src/protocol/protocol.hpp"
#include "../test.hpp"
#include <utility>

inline std::string parser_result_to_string(const protocol::ParserResult& l)
{
  switch (l)
  {
  case protocol::ParserResult::NeedMoreData:
    return "NeedMoreData";
  case protocol::ParserResult::Invalid:
    return "Invalid";
  case protocol::ParserResult::Complete:
    return "Complete";
  default:
    std::unreachable();
  }
}

inline void assert_equal(const protocol::ParserResult& l, const protocol::ParserResult& r)
{
  if (l != r) throw test::ComparisonException(parser_result_to_string(l), parser_result_to_string(r));
}

inline void assert_equal(protocol::HTTP::Method& l, protocol::HTTP::Method r)
{
  if (l != r)
    throw test::ComparisonException(protocol::HTTP::method_to_string(l), protocol::HTTP::method_to_string(r));
}
