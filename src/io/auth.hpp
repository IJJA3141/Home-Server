#pragma once

#include "../config.hpp"

#include <chrono>
#include <filesystem>
#include <uuid/uuid.h>

struct session
{
  std::chrono::system_clock::time_point valid_until;
  std::string user;
  uuid_t uuid;
};

std::ostream& operator<<(std::ostream& _ostream, const session& _session);

template <std::size_t _N> class session_cache
{
public:
  /**
   * @brief Constructs a session cache.
   *
   * Initializes the session cache with a given storage path and session time-to-live.
   *
   * @param _cache_path The path of the file used to store sessions.
   * @param _ttl The time-to-live duration for each session.
   */
  session_cache(const std::filesystem::path _cache_path, const std::chrono::seconds _ttl);

  /**
   * @brief Creates a new session and stores it.
   *
   * It caches the new usser and stores it in `_cache_path`.
   *
   * @param _user The name of the new user. It must not be empty or contain \n.
   *
   * @return The newly created session.
   */
  const session generate(const std::string _user);

  /**
   * @brief Fetches the session with the matching UUID.
   *
   * If no session with a matching UUID is cached, searches through the sessions stored in `_cache_path`
   * and purges any invalid sessions.
   *
   * @param _uuid The UUID to match.
   *
   * @return If found, returns the index of the session whose UUID matches `_uuid`. If not found, returns -1.
   */
  int fetch(const uuid_t _uuid);

  const session operator[](const std::size_t _index) const;

private:
  std::filesystem::path write_path_;
  std::filesystem::path read_path_;

  const std::chrono::seconds ttl_;
  session sessions_[_N];

  inline std::size_t hash(const uuid_t& _uuid);
};
