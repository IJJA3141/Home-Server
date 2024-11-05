#pragma once

#include "../log.hpp"

namespace test
{

void request();
void router();
void stream();
void cache();

inline void all()
{
  stream();
  request();
  router();
  cache();
  return;
};

}; // namespace test
