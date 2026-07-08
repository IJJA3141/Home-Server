#pragma once

#include <compare>
#include <format>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <uuid/uuid.h>

class UuidView;

/**
 * Simple C++ wrapper around libuuid.
 *
 * Provides a small value-type class for working with UUIDs while hiding the
 * C-style libuuid API. Supports parsing, generation, comparison, and
 * conversion to string.
 */
class Uuid
{
public:
  // remove \0 from length
  static constexpr size_t UNPARSED_SIZE = UUID_STR_LEN - 1;
  static constexpr size_t PARSED_SIZE = 16;

  /**
   *  @brief Parse a UUID from its string representation.
   *  @param s  UUID string (typically "%08x-%04x-%04x-%04x-%012x").
   *  @throws std::invalid_argument if the string is not a valid UUID.
   *  @return Parsed Uuid object.
   */
  [[nodiscard]] static Uuid parse(std::span<const char, UNPARSED_SIZE> s)
  {
    uuid_t uuid;
    if (uuid_parse(s.data(), uuid)) throw std::invalid_argument(std::format("invalid uuid {}", s));

    return uuid;
  }

  /**
   * @brief Parse a UUID from a string without throwing exceptions.
   * @param s  UUID string.
   * @return std::nullopt if parsing fails, otherwise the parsed Uuid.
   */
  [[nodiscard]] static std::optional<Uuid> parse_safe(std::span<const char, UNPARSED_SIZE> s) noexcept
  {
    uuid_t uuid;
    if (uuid_parse(s.data(), uuid)) return std::nullopt;
    return uuid;
  }

  /**
   * @brief Generate a new random UUID using libuuid.
   * @return Newly generated Uuid.
   */
  [[nodiscard]] static Uuid generate() noexcept
  {
    uuid_t uuid;
    uuid_generate(uuid);
    return uuid;
  }

  /**
   * Construct a Uuid from a raw libuuid uuid_t.
   * Copies the underlying UUID value.
   */
  Uuid(const uuid_t uuid) noexcept { uuid_copy(this->uuid_, uuid); };

  Uuid(const Uuid& uuid) = default;
  Uuid(Uuid&&) = default;
  Uuid& operator=(const Uuid&) = default;
  Uuid& operator=(Uuid&&) = default;

  /**
   * Convert UUID to its canonical string representation.
   * Enables implicit conversion to std::string.
   */
  [[nodiscard]] operator std::string() const noexcept
  {
    char out[UUID_STR_LEN];
    uuid_unparse(this->uuid_, out);
    return out;
  }

  // Comparison operators
  [[nodiscard]] bool operator==(const Uuid& that) const noexcept { return !uuid_compare(this->uuid_, that.uuid_); }
  [[nodiscard]] std::strong_ordering operator<=>(const Uuid& that) const noexcept
  {
    int r = uuid_compare(this->uuid_, that.uuid_);
    if (r < 0) return std::strong_ordering::less;
    if (r > 0) return std::strong_ordering::greater;
    return std::strong_ordering::equal;
  }

protected:
  uuid_t uuid_;
  friend std::hash<Uuid>;
  friend UuidView;
};

struct UuidView
{
  UuidView(const Uuid& uuid) : _M_ptr_{uuid.uuid_} {}
  UuidView(const unsigned char* src) : _M_ptr_{src} {}

  // Comparison operators
  [[nodiscard]] bool operator==(const UuidView& that) const noexcept
  {
    return !uuid_compare(this->_M_ptr_, that._M_ptr_);
  }

  [[nodiscard]] std::strong_ordering operator<=>(const UuidView& that) const noexcept
  {
    int r = uuid_compare(this->_M_ptr_, that._M_ptr_);
    if (r < 0) return std::strong_ordering::less;
    if (r > 0) return std::strong_ordering::greater;
    return std::strong_ordering::equal;
  }

  /**
   * Convert UUID to its canonical string representation.
   * Enables implicit conversion to std::string.
   */
  [[nodiscard]] operator std::string() const noexcept
  {
    char out[UUID_STR_LEN];
    uuid_unparse(this->_M_ptr_, out);
    return out;
  }

protected:
  const unsigned char* _M_ptr_;
  friend std::hash<UuidView>;
};

template <> struct std::hash<Uuid>
{
  std::size_t operator()(const Uuid& uuid) const noexcept
  {
    return std::hash<std::string_view>{}(
        std::string_view{reinterpret_cast<const char*>(uuid.uuid_), sizeof(uuid_t)});
  }
};

template <> struct std::hash<UuidView>
{
  std::size_t operator()(const UuidView& uuid) const noexcept
  {
    return std::hash<std::string_view>{}(
        std::string_view{reinterpret_cast<const char*>(uuid._M_ptr_), sizeof(uuid_t)});
  }
};
