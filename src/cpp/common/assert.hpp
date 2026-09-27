#pragma once

#include "ansi.hpp"

#include <exception>
#include <format>
#include <print>
#include <source_location>
#include <string>

struct failed_assertion : public std::exception
{
  failed_assertion(std::string what) : what_(what) {};
  const char* what() const noexcept override { return what_.c_str(); }

private:
  const std::string what_;
};

namespace detail
{

[[noreturn]] inline void assertion_failed(std::source_location from, std::string_view expression,
                                          std::string_view message = "")
{
#ifdef __TEST
  throw failed_assertion(std::format("expression " ESC HRED "{}" ESC RST " was evaluated to false" ESC HYELLOW
                                     "{}" ESC RST "\n\t   from file " ESC HRED "{}" ESC RST
                                     " in function " ESC HRED "{}" ESC RST " at line " ESC HRED "{}" ESC RST "\n",
                                     expression, message, from.file_name(), from.function_name(), from.line()));

#else
  std::println(stderr,
               "[" ESC HRED "assertion failed" ESC RST "]" ESC YELLOW "{}" ESC RST "\n   expression:  " ESC RED
               "{}" ESC RST "\n   location:    {}:{}\n   function:    {}\n",
               message, expression, from.file_name(), from.line(), from.function_name());
  abort();
#endif
}

} // namespace detail

#ifdef __DEBUG
#define assert(expression, ...)                                                                                   \
  if (!(expression))                                                                                              \
    detail::assertion_failed(std::source_location::current(),                                                     \
                             #expression __VA_OPT__(, ": " + std::format(__VA_ARGS__)));
#else
#define assert(...) (void)(0)
#endif
