#pragma once

#include <chrono>
#include <filesystem>
#include <uuid/uuid.h>

#include "../config.hpp"

enum error
{
  CLOSED
};

struct session_id
{
  std::chrono::system_clock::time_point valid_until;
  std::string user;
  uuid_t uuid;
};

class session_cache
{
public:
  session_cache(std::filesystem::path _cache_path);

  const session_id generate(const std::string _user);
  int fetch(const uuid_t uuid);

  const session_id operator[](const size_t _index) const;

  // private:
  session_id cache_[session_cache_size];
  std::filesystem::path cache_path_tmp_;
  std::filesystem::path cache_path_;
};

std::ostream& operator<<(std::ostream& _ostream, const session_id& _id);
