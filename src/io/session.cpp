#include "../log.hpp"
#include "auth.hpp"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <uuid/uuid.h>

#define FMT "%D:%T"

constexpr auto elaps = 24 * 3;

std::ostream& operator<<(std::ostream& _ostream, const session_id& _id)
{
  char uuid[37];
  uuid_unparse(_id.uuid, uuid);
  return _ostream << uuid << std::format("{:" FMT "}", _id.valid_until) << _id.user;
}

session_cache::session_cache(std::filesystem::path _cache_path) : cache_path_(_cache_path)
{
  this->cache_path_tmp_ = this->cache_path_;
  this->cache_path_tmp_.replace_filename("." + std::string(this->cache_path_.filename()));
  unsigned char null[37];
  this->fetch(null);

  return;
}

const session_id session_cache::generate(const std::string _user)
{
  assert(!_user.empty(), "attempted to create an empty user");

  session_id id = {std::chrono::system_clock::now() + std::chrono::hours{elaps}, _user};
  uuid_generate(id.uuid);

  std::ofstream stream(this->cache_path_, std::ios::app);
  stream << id << "\n";
  stream.close();
  assert(!stream.fail(), "could not save generated uuid");

  this->cache_[id.uuid[0] % session_cache_size] = id;
  return id;
}

int session_cache::fetch(const uuid_t _uuid)
{
  std::chrono::time_point now = std::chrono::system_clock::now();
  std::ofstream out(this->cache_path_tmp_, std::ios::out | std::ios::trunc);
  std::ifstream in(this->cache_path_);
  int index = -1;
  session_id id;
  char buff[37];

  assert(in.is_open());
  while (in.read(buff, 36))
  {
    buff[36] = '\0';
    assert(!uuid_parse(buff, id.uuid), "could not parse uuid: " + std::string(buff));
    in >> std::chrono::parse(FMT, id.valid_until);
    std::getline(in, id.user);

    if (id.valid_until > now)
    {
      out << id << std::endl;
      this->cache_[id.uuid[0] % session_cache_size] = id;
      if (!uuid_compare(_uuid, id.uuid)) index = id.uuid[0] % session_cache_size;
    }
  }

  in.close();
  out.close();
  assert(!out.fail(), "failed to save cache");

  std::filesystem::rename(this->cache_path_tmp_, this->cache_path_);

  return index;
}

const session_id session_cache::operator[](const size_t _index) const
{
  assert(_index < session_cache_size, "Assertion '_index < session_cache_size' failed.");
  return this->cache_[_index];
}
