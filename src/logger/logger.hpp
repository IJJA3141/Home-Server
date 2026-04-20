#pragma once

#include <format>
#include <print>
#include <string>
#include <string_view>
#include <utility>

class Logger
{
public:
  enum Level
  {
    INFO,
    WARN,
    ERROR,
    CRIT
  };

  static Logger& New()
  {
    static Logger instance;
    return instance;
  }

  template <typename... Args> void log(const Level lvl, std::format_string<Args...> fmt, Args&&... args) const
  {
    const std::string s = std::format(fmt, std::forward<Args>(args)...);

    switch (lvl)
    {
    case INFO:
      std::println("[info]\t{}", s);
      break;
    case WARN:
      std::println("[\x1b[33mwarn\x1b[0m]\t{}", s);
      break;
    case ERROR:
      std::println("[\x1b[31merror\x1b[0m]\t{}", s);
      break;
    case CRIT:
      std::println("[\x1b[38mcrit\x1b[0m]\t{}", s);
      break;
    }
  }

  template <typename... Args> void info(std::format_string<Args...> fmt, Args&&... args) const
  {
    log(INFO, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> void warn(std::format_string<Args...> fmt, Args&&... args) const
  {
    log(WARN, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> void error(std::format_string<Args...> fmt, Args&&... args) const
  {
    log(ERROR, fmt, std::forward<Args>(args)...);
  }

  template <typename... Args> void crit(std::format_string<Args...> fmt, Args&&... args) const
  {
    log(CRIT, fmt, std::forward<Args>(args)...);
  }

  void info(std::string_view s) const { log(INFO, "{}", s); }
  void warn(std::string_view s) const { log(WARN, "{}", s); }
  void error(std::string_view s) const { log(ERROR, "{}", s); }
  void crit(std::string_view s) const { log(CRIT, "{}", s); }

private:
  Logger() = default; // Private constructor for singleton
};
