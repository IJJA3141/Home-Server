#pragma once

// from https://en.cppreference.com/cpp/utility/variant/visit
template <class... Ts> struct overloads : Ts...
{
  using Ts::operator()...;
};
