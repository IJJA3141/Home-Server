#pragma once

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

template <typename T> inline void log(const T& _msg) { std::clog << "[log] " << std::format("{}", _msg) << std::endl; }

template <typename T> inline void err(const T& _msg)
{
  std::clog << "\x1b[31m[error] " << std::format("{}", _msg) << "\x1b[0m" << std::endl;
}

template <typename T> inline void warn(const T& _msg)
{
  std::clog << "\x1b[33m[warning] " << std::format("{}", _msg) << "\x1b[0m" << std::endl;
}

#ifdef DEBUG
template <typename T> inline void debugf(const T& _msg)
{
  std::clog << "\x1b[34m[debug] " << std::format("{}", _msg) << "\x1b[0m" << std::endl;
};

template <typename T> inline void debug(const T& _msg)
{
  std::clog << "\x1b[34m[debug] " << _msg << "\x1b[0m" << std::endl;
};
#else
template <typename T> inline void debugf(T _msg) {};
template <typename T> inline void debug(T _msg) {};
#endif

template <typename T> void assert(bool _assertion, const T& _msg)
{
  if (_assertion) return;

  std::clog << "\x1b[38;5;196m[fatal] " << std::format("{}", _msg) << "\x1b[0m\n" << std::endl;
  exit(1);
}

inline void assert(bool _assertion) { assert(_assertion, strerror(errno)); }
