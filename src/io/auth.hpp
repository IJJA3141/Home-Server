#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <uuid/uuid.h>

#include "../config.hpp"

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

private:
  session_id cache_[session_cache_size];
  std::filesystem::path cache_path_;
  void add(session_id id);
};
