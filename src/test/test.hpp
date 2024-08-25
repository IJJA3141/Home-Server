#pragma once

#include "../log.hpp"

namespace test
{

void request();
void router();
void stream();

inline void all()
{
  stream();
  request();
  router();
  return;
};

}; // namespace test
