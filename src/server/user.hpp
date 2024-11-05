#pragma once

uuencode

#include <string>
struct User{
  std::string name;
  std::string slat;

  // sha 256 -> 256 bits
  // cpp char -> 1 byte = 8bits
  // 256 / 8 = 32
  const char hash[32];
};
