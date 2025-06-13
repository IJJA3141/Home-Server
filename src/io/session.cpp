#include "../log.hpp"
#include "auth.hpp"

#include <fstream>

#ifndef SESSION_TIME_FMT
#define SESSION_TIME_FMT "%D:%T"
#endif

std::ostream& operator<<(std::ostream& _ostream, const session& _session)
{
  char uuid[37];
  uuid_unparse(_session.uuid, uuid);
  return _ostream << uuid << std::format("{:" SESSION_TIME_FMT "}", _session.valid_until) << _session.user;
}

template <std::size_t _N>
session_cache<_N>::session_cache(std::filesystem::path _cache_path, std::chrono::seconds _ttl)
    : read_path_(_cache_path), write_path_(_cache_path), ttl_(_ttl)
{
  this->write_path_.replace_filename("." + std::string(_cache_path.filename()));

  // initialize cache
  unsigned char null[37];
  this->fetch(null);

  return;
}

template <std::size_t _N> std::size_t session_cache<_N>::hash(const uuid_t& _uuid) { return _uuid[0] % _N; }

template <std::size_t _N> const session session_cache<_N>::generate(const std::string _user)
{
  // sanitization
  assert(!_user.empty(), "empty user name.");
  assert(_user.find("\n") == _user.npos, "\\n in user name.");

  // generate session
  session session = {std::chrono::system_clock::now() + this->ttl_, _user};
  uuid_generate(session.uuid);

  // store session in file
  std::ofstream cache(this->read_path_, std::ios::app);
  cache << session << "\n";
  cache.close();
  assert(!cache.fail(), "could not save generated session.");

  // store session in cache
  this->sessions_[this->hash(session.uuid)] = session;

  return session;
}

template <std::size_t _N> int session_cache<_N>::fetch(const uuid_t _uuid)
{
  std::chrono::time_point now = std::chrono::system_clock::now();

  // search for match in cache
  for (int i = 0; i < _N; ++i)
    if (now < this->sessions_[i].valid_until && !uuid_compare(_uuid, this->sessions_[i].uuid)) return i;

  // open read and write files
  std::ofstream out(this->write_path_, std::ios::out | std::ios::trunc);
  std::ifstream in(this->read_path_);
  assert(in.is_open() && out.is_open());

  session session;
  char buffer[37];
  int index = -1;
  buffer[36] = '\0';

  while (in.read(buffer, 36))
  {
    // read session
    assert(!uuid_parse(buffer, session.uuid), "could not parse uuid: " + std::string(buffer));
    in >> std::chrono::parse(SESSION_TIME_FMT, session.valid_until);
    assert(!in.fail(), "could not parse life time");
    std::getline(in, session.user);
    assert(!in.fail(), "could not parse user name");

    if (now < session.valid_until)
    {
      // store session if valid
      out << session << std::endl;
      this->sessions_[this->hash(session.uuid)] = session;

      // set index to matching session
      if (!uuid_compare(_uuid, session.uuid)) index = this->hash(session.uuid);
    }
  }

  // save cache
  in.close();
  out.close();
  assert(!out.fail(), "failed to save cache");

  std::filesystem::rename(this->write_path_, this->read_path_);

  return index;
}

template <std::size_t _N> const session session_cache<_N>::operator[](const std::size_t _index) const
{
  assert(_index < _N, "Assertion '_index < _N' failed.");
  return this->sessions_[_index];
}

template class session_cache<session_cache_size>;
