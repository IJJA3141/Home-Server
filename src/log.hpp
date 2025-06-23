#pragma once

#include <cstring>
#include <format>
#include <iostream>
#include <type_traits>

enum Level
{
  DBG,
  LOG,
  WARN,
  ERR
};

// template <typename... T> constexpr void debug(const T&...);

// template <typename... T> constexpr void log(const T&...);
// template <typename... T> constexpr void warn(const T&...);
// template <typename... T> constexpr void err(const T&...);

// template <typename... T> constexpr void assert(const bool&& _assertion, const T&...);
// template <typename... T> constexpr bool check(const Level, const bool&&  _check, const T&...);

template <typename T> constexpr void fmt(std::ostream& _ostream, const T& _)
{

  if constexpr (std::is_default_constructible_v<std::formatter<T, char>>) _ostream << std::format("{}", _) << " ";
  else _ostream << _ << " ";
};

#ifdef DEBUG
template <typename... T> constexpr void debug(const T&... _)
{
  static_assert(sizeof...(T) != 0, "If you have nothing to say, shut up.");

  std::clog << "\x1b[34m[debug]\x1b[0m\t";
  (fmt(std::clog, _), ...);
  std::clog << std::endl;
}
#else
template <typename... T> constexpr void debug(const T&... _) {}
#endif

template <typename... T> constexpr void log(const T&... _)
{
  static_assert(sizeof...(T) != 0, "If you have nothing to say, shut up.");

  std::clog << "[log]\t";
  (fmt(std::clog, _), ...);
  std::clog << std::endl;
}

template <typename... T> constexpr void warn(const T&... _)
{
  static_assert(sizeof...(T) != 0, "If you have nothing to say, shut up.");

  std::clog << "\x1b[33m[warning]\x1b[0m\t";
  (fmt(std::clog, _), ...);
  std::clog << std::endl;
}

template <typename... T> constexpr void err(const T&... _)
{
  static_assert(sizeof...(T) != 0, "If you have nothing to say, shut up.");

  std::clog << "\x1b[31m[error]\x1b[0m\t";
  (fmt(std::clog, _), ...);
  std::clog << std::endl;
}

template <typename... T> constexpr void assert(bool _assertion, const T&... _)
{
  if (!_assertion)
  {
    std::clog << "\x1b[38;5;196m[fatal]\t";
    if constexpr (sizeof...(T) == 0) fmt(std::clog, strerror(errno));
    else (fmt(std::clog, _), ...);
    std::clog << std::endl;
    exit(1);
  }
}

template <typename... T> constexpr bool check(Level _level, const bool&& _check, const T&... _)
{
  if (_check) return true;

  switch (_level)
  {
  case DBG:
    debug(_...);
    break;
  case LOG:
    log(_...);
    break;
  case WARN:
    warn(_...);
    break;
  case ERR:
    err(_...);
    break;
  }

  return false;
}
