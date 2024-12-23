#pragma once

#include "../log.hpp"

namespace test
{

void request();
void router();
void stream();
void cache();
void login();

inline void all()
{
  stream();
  request();
  router();
  cache();
  login();
  return;
};

}; // namespace test
