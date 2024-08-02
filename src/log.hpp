#pragma once

#include <iostream>

#define LOG(STR) std::cout << "[log] " << STR << std::endl;
#define WARN(STR) std::cout << "\x1b[33m[warn] " << STR << "\x1b[0m" << std::endl;
#define ERR(STR) std::cout << "\x1b[31m[error] " << STR << "\x1b[0m" << std::endl;

#ifdef DEBUG
#define PRINT(STR) std::cout << "[debug] " << STR << std::endl;
#else
#define PRINT(STR)
#endif // DEBUG

#define LOGV(V)                                                                                    \
  std::cout << "[log]";                                                                            \
  if (V.size() == 1) {                                                                             \
    for (const auto &arg : V)                                                                      \
      std::cout << "\t" << arg << "\n";                                                            \
  } else {                                                                                         \
    std::cout << "\t{}" << std::endl;                                                              \
  }                                                                                                \
  std::cout << "\x1b[0m";

#define WARNV(V)                                                                                   \
  std::cout << "\x1b[33m[warn]";                                                                   \
  if (V.size() != 0) {                                                                             \
    for (const auto &arg : V)                                                                      \
      std::cout << "\t" << arg << "\n";                                                            \
  } else {                                                                                         \
    std::cout << "\t{}" << std::endl;                                                              \
  }                                                                                                \
  std::cout << "\x1b[0m";
#define ERRV(V)                                                                                    \
  std::cout << "\x1b[31m[error]";                                                                  \
  if (V.size() != 0) {                                                                             \
    for (const auto &arg : V)                                                                      \
      std::cout << "\t" << arg << "\n";                                                            \
  } else {                                                                                         \
    std::cout << "\t{}" << std::endl;                                                              \
  }                                                                                                \
  std::cout << "\x1b[0m";

#ifdef DEBUG
#define PRINTV(V)                                                                                  \
  std::cout << "[debug]";                                                                          \
  if (V.size() != 0) {                                                                             \
    for (const auto &arg : V)                                                                      \
      std::cout << "\t" << arg << "\n";                                                            \
  } else {                                                                                         \
    std::cout << "\t{}" << std::endl;                                                              \
  }
#else
#define PRINTV(V)
#endif // DEBUG
