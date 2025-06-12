#include "../log.hpp"
#include "auth.hpp"

#include <chrono>
#include <format>
#include <fstream>
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

const session_id session_cache::fetch(const uuid_t _uuid)
{
  std::chrono::time_point now = std::chrono::system_clock::now();
  std::fstream in(this->cache_path_);
  std::stringstream out;
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
      if (!uuid_compare(_uuid, id.uuid)) break;
    }
  }

  in.clear();
  in.seekp(0);
  in << out.str();
  in.close();

  assert(!in.fail(), "failed to save cache");
  return id;
}
