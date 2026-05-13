#pragma once

#include <exception>
#include <format>
#include <stdexcept>

struct configuration_error : std::exception
{
  template <typename... Args>
  constexpr inline explicit configuration_error(std::format_string<Args...> _fmt, Args&&... _args)
      : msg_(std::format(_fmt, std::forward<Args>(_args)...)) {};

  constexpr inline const char* what() const noexcept override { return msg_.c_str(); }

private:
  const std::string msg_;
};

struct initialisation_error : std::runtime_error
{
  template <typename... Args>
  constexpr inline explicit initialisation_error(std::format_string<Args...> _fmt, Args&&... _args)
      : runtime_error(std::format(_fmt, std::forward<Args>(_args)...)) {};
};


struct malformed_route : std::invalid_argument
{
  template <typename... Args>
  constexpr inline explicit malformed_route(std::format_string<Args...> _fmt, Args&&... _args)
      : invalid_argument(std::format(_fmt, std::forward<Args>(_args)...)) {};
};
