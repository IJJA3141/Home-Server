#pragma once

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

/**
 *  @brief  Computes decoded byte size for a Base64 string.
 *
 *  Validates structural constraints:
 *  - length must be a multiple of 4
 *  - padding characters `=` must not exceed 2
 *
 *  @return decoded size, or std::nullopt if invalid structure
 */
constexpr std::optional<size_t> base64_size(std::string_view base64) noexcept
{
  if (base64.empty()) return 0;
  if (base64.size() & 0b11) return std::nullopt;

  size_t n = std::count(base64.end() - 4, base64.end(), '=');
  return n <= 2 ? std::make_optional(((base64.size() * 3) >> 2) - n) : std::nullopt;
}

/**
 *  @brief  Decodes Base64 into a preallocated output buffer.
 *
 *  Input is first mapped into 6-bit values in-place, then expanded into bytes.
 *
 *  @note @a _decoded must already be correctly sized
 *
 *  @return  true on success, false on invalid character
 */
constexpr bool btoa(std::string _encoded, std::string& _decoded) noexcept
{
  for (char& c : _encoded)
  {
    if ('a' <= c && c <= 'z') c -= 71;
    else if ('A' <= c && c <= 'Z') c -= 65;
    else if ('0' <= c && c <= '9') c += 4;
    else if (c == '+') c = 62;
    else if (c == '/') c = 63;
    else if (c == '=') c = 0;
    else return false;
  }

  std::string::iterator src = _encoded.begin();
  for (std::string::iterator dst = _decoded.begin(); dst < _decoded.end();)
  {
    *dst++ = (*src++ << 2 & 0b11111100) | (*src >> 4 & 0b00000011);
    *dst++ = (*src++ << 4 & 0b11110000) | (*src >> 2 & 0b00001111);
    *dst++ = (*src++ << 6 & 0b11000000) | (*src++ & 0b00111111);
  }

  return true;
}

/**
 *  @brief  Decodes a Base64 string.
 *
 *  @throws std::invalid_argument on:
 *  - invalid length / padding
 *  - invalid Base64 characters
 *
 *  @return  Decoded string
 */
constexpr std::string base64_decode(const std::string& base64)
{
  std::optional<size_t> size = base64_size(base64);
  if (!size)
    throw std::invalid_argument(
        "base64 size should be a multiple of 4 and have no more than tow padding characters");

  if (size.value() == 0) return "";

  std::string out(size.value(), ' ');
  if (!btoa(base64, out)) throw std::invalid_argument("invalid character in base64");
  return out;
}

namespace safe
{

/**
 *  @brief  Decodes Base64 without exceptions.
 *  @return  Decoded string, or std::nullopt on any validation or decoding failure.
 */
constexpr std::optional<std::string> base64_decode(const std::string& base64) noexcept
{
  std::optional<size_t> size = base64_size(base64);
  if (!size) return std::nullopt;
  if (size.value() == 0) return "";

  std::string out(size.value(), ' ');
  if (!btoa(base64, out)) return std::nullopt;
  return out;
}

} // namespace safe
