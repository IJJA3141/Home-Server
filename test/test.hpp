#pragma once

#include <format>
#include <print>

namespace test
{

struct ComparisonException : std::exception
{
  template <typename T>
  constexpr inline explicit ComparisonException(T&& got, T&& expected)
      : reason(std::format("got {}, expected {}", got, expected))
  {
  }
  const std::string reason;
  const char* what() const noexcept override { return this->reason.c_str(); }
};

template <typename T> constexpr void assert_equal(const T&& l, const T&& r)
{
  if (l != r) throw ComparisonException(l, r);
}

} // namespace test

#define CATCH(test_name, x)                                                                                       \
  catch (x e)                                                                                                     \
  {                                                                                                               \
    std::println("\x1b[31m  - {} threw an instance of '{}'.\n\x1b[0m    {}\n", test_name, #x, e.what());          \
    _exception_caught = true;                                                                                     \
  }

#define SECTION(section_name)                                                                                     \
  std::println("\x1b[36m{}:\x1b[0m", section_name);                                                               \
  // _i = 0, _j = 0;

#define REQUIRE(test_name, test_function)                                                                         \
  _i++;                                                                                                           \
  _exception_caught = false;                                                                                      \
  try                                                                                                             \
  {                                                                                                               \
    test_function                                                                                                 \
  }                                                                                                               \
  CATCH(test_name, test::ComparisonException)                                                                     \
  CATCH(test_name, std::runtime_error)                                                                            \
  CATCH(test_name, std::logic_error)                                                                              \
  CATCH(test_name, std::exception)                                                                                \
  if (!_exception_caught)                                                                                         \
  {                                                                                                               \
    std::println("\x1b[32m  + {}\x1b[0m", test_name);                                                             \
    ++_j;                                                                                                         \
  };

#define REQUIRE_THROW(test_name, exception_type, test_function)                                                   \
  _i++;                                                                                                           \
  _exception_caught = false;                                                                                      \
  try                                                                                                             \
  {                                                                                                               \
    test_function                                                                                                 \
  }                                                                                                               \
  catch (exception_type & e)                                                                                      \
  {                                                                                                               \
    std::println("\x1b[32m  + {} threw an instance of '{}'.\x1b[0m", test_name, #exception_type);                 \
    ++_j;                                                                                                         \
    _exception_caught = true;                                                                                     \
  }                                                                                                               \
  CATCH(test_name, ComparisonException)                                                                           \
  CATCH(test_name, std::exception)                                                                                \
  if (!_exception_caught)                                                                                         \
  {                                                                                                               \
    std::println("\x1b[31m  - {} should have thrown a(n) '{}'.\x1b[0m", test_name, #exception_type);              \
  };

#define ENDSECTION                                                                                                \
  if (_i != _j)                                                                                                   \
  {                                                                                                               \
    std::println("[\x1b[31mfailure\x1b[0m] Total: {}, Passed: {}, Failed {}\n", _i, _j, _i - _j);                 \
    _flag = _i - _j;                                                                                              \
  }                                                                                                               \
  else std::println("[\x1b[32msuccess\x1b[0m]\n");

#define _REQUIRE(test_function) REQUIRE(std::format("test {}", _i), test_function);
