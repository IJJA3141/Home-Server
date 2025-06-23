#pragma once

#include <chrono>
#include <filesystem>
#include <ostream>
#include <string_view>
#include <uuid/uuid.h>

struct Session
{
  std::chrono::system_clock::time_point valid_until;
  std::string user;
  uuid_t uuid;

  friend std::ostream& operator<<(std::ostream& _ostream, const Session& _session);
};

class AuthAgent
{
public:
  const std::filesystem::path write_path;
  const std::filesystem::path read_path;
  const std::filesystem::path user_dir;

  /// TODO change -v
  /**
   * @brief Constructs a session cache.
   *
   * Initializes the session cache with a given storage path and session time-to-live.
   *
   * @param _cache_path The path of the file used to store sessions.
   * @param _ttl The time-to-live duration for each session.
   */
  AuthAgent(const size_t _size, const std::filesystem::path _data_dir, const std::chrono::seconds _ttl);

  /**
   * @brief Creates a new session and stores it.
   *
   * It caches the new usser and stores it in `_cache_path`.
   *
   * @param _user The name of the new user. It must not be empty or contain \n.
   *
   * @return The newly created session.
   */
  const Session generate(const std::string _user);

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

  /**
   * @return if the _password is invalide.
   */
  bool invalidate_password(const std::string_view _user, const std::string& _password) const;
  void save_password_hash(const std::string_view _user, const std::string& _password);

  const Session operator[](const size_t _index) const;

private:
  const std::chrono::seconds ttl_;
  Session* const sessions_;
  const size_t size_;

  inline size_t hash_uuid(const uuid_t& _uuid) const noexcept;
  static void user_sanitization(const std::string_view _user);
};
