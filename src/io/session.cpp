#include "auth.hpp"

#include <cassert>
#include <chrono>
#include <format>
#include <fstream>
#include <ios>
#include <string>
#include <uuid/uuid.h>

constexpr auto format = "{:%D%T}";

const session_id session_cache::generate(const std::string user)
{
  session_id id = {std::chrono::system_clock::now() + std::chrono::hours{24 * 3}, user};
  uuid_generate(id.uuid);

  std::ofstream stream(this->cache_path_, std::ios::app);
  assert(stream.is_open());

  stream << id.uuid << "\n";
  stream << std::format(format, id.valid_until) << "\n";
  stream << id.user << "\n";

  stream.close();

  // this->add(id);

  return id;
}

session_cache::session_cache(std::filesystem::path _cache_path) : cache_path_(_cache_path)
{
  std::ifstream stream(this->cache_path_);
  assert(stream.is_open());

  std::string in;
  int i = 0;

  while (stream >> in && i < session_cache_size)
  {
    this->cache_[i] = session_id();

    this->cache_[i].user = in;
    stream >> std::chrono::parse(format, this->cache_[i].valid_until);
    stream >> in;
    uuid_parse(in.c_str(), this->cache_[i].uuid);

    i++;
  }

  stream.close();

  return;
}

void session_cache::add(session_id id)
{
  for (int i = 0; i < session_cache_size; i++)
  {
    if (this->cache_[i].valid_until < std::chrono::system_clock::now())
    {
      this->cache_[i] = id;
      return;
    }
  }
}
